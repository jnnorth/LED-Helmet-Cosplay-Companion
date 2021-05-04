#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <chrono>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "led-matrix.h"
#include "pixel-mapper.h"
#include <Magick++.h>
#include <wiringPi.h>

#define MIN_BLINK_FRAMES          10
#define MAX_BLINK_FRAMES          20
#define MIN_FRAMES_BETWEEN_BLINKS 100
#define MAX_FRAMES_BETWEEN_BLINKS 200
#define MAX_FRAME_COUNT           100000


int map[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
bool buttons_real[10];
bool buttons_virtual[10];
bool mouth_open;

int color_red;
int color_green;
int color_blue;
using namespace Magick;
using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

#define RECEIVER_1 0
#define	RECEIVER_2 1
#define DRAWING    2
#define RECEIVER_3 3
#define FAN        4
#define NUM_PROCESSES 5
pid_t process[NUM_PROCESSES];
pid_t face_1_pid;
pid_t face_2_pid;
bool isChildDrawer = false;
volatile bool interrupted = false;

int curr_frame;
int blink_frames_left;
int next_blink_frame;
bool is_blinking;

int my_argc;
char ** my_argv;

float temperature;
float humidity;

// Interrupt handler, closes drawing process and receivers
void catch_int(int sig_num) {
	if(isChildDrawer) {
		exit(0);
	}
	for(int i = 0; i < NUM_PROCESSES; i++) {
		if(process[i] != 0) {
			kill(process[i], sig_num);
		}
	}
	exit(0);
}

void catch_int_child(int sig_num) {
	interrupted = true;
}

void catch_int_face(int sig_num) {
	interrupted = true;
}

int random_int_on_range(int min, int max) {
	return min + (rand() % (max - min));
}

// I am unsure if this function will be used
// Transforms the button_virtual array into a 10-bit number and returns
// the number as an int
int buttons_to_int() {
	int ret = 0;
	for(int i = 0; i < 10; i++) {
		if(buttons_virtual[i]) {
			ret |= (1 << i);
		}
	}
	return ret;
}

// Returns the time in ms since the epoch in milliseconds mod 100
int get_ms_time() {
	using namespace std::chrono;
	uint64_t ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return (int)(ms_time % 100);
}
/*
void build_stream(std::string file) {
	pid_t fork_ret = fork();
	if(fork_ret == 0) {
	}
}
*/
// Waits until the python processes are guarunteed to not be writing into files
void wait_to_read() {
	while((20 < get_ms_time()) || (get_ms_time() > 80)) {
		// Do nothing, sleep to reduce CPU usage
		usleep(10000);
	}
}

void exec_gif_drawer(std::string file) {
	file += ".stream";
	std::string img_viewer   = "utils/led-image-viewer";
	std::string hardware_pulse_flag = "--led-no-hardware-pulse";
	char img_viewer_c[img_viewer.size()+1];
	char file_c[file.size()+1];
	char pulse_c[hardware_pulse_flag.size()+1];
	strcpy(img_viewer_c, img_viewer.c_str());
	strcpy(file_c, file.c_str());
	strcpy(pulse_c, hardware_pulse_flag.c_str());
	int fd = open("/dev/null", O_WRONLY | O_CREAT, 0666);
	dup2(fd, 1);
	dup2(fd, 2);
	char * args[] = {
		//sudo_c, 
		img_viewer_c, file_c, pulse_c, NULL};
//	execvp(args[1], args);
	execvp(args[0], args);
	close(fd);
}


void draw_image(std::string file, bool isFace = true) {
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	if(isFace) {
		sa.sa_handler = catch_int_face;
	}
	else {
		sa.sa_handler = catch_int_child;
	}
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);

	// TODO: WHEN BLINKING FACES ADDED, REMOVE THIS LINE
	// *************************************************************************************************************************************************
	// *************************************************************************************************************************************************
	isFace = false;	
	// *************************************************************************************************************************************************	
	// *************************************************************************************************************************************************	

	if(isFace) {
		std::string file2 = file.substr(0, file.size()-4);
		file2 += "_blink.png";
		int frame = 0;
		int switch_frame = random_int_on_range(MIN_FRAMES_BETWEEN_BLINKS,MAX_FRAMES_BETWEEN_BLINKS);
		bool blink = false;
		bool redraw = false;
		face_1_pid = 0;
		face_2_pid = 0;
		while(!interrupted) {
			if(redraw) {
				redraw = false;
				if(!blink) {
					face_1_pid = fork();
					if(face_1_pid == 0) {
						exec_gif_drawer(file);
					}
					if(face_2_pid != 0) {
						kill(face_2_pid, SIGINT);
					}
				}
				else {
					face_2_pid = fork();
					if(face_2_pid == 0) {
						exec_gif_drawer(file2);
					}
					if(face_1_pid != 0) {
						kill(face_1_pid, SIGINT);
					}
				}
			}
			frame++;
			if(switch_frame <= frame) {
				frame = 0;
				if(blink) {
					switch_frame = random_int_on_range(MIN_FRAMES_BETWEEN_BLINKS, MAX_FRAMES_BETWEEN_BLINKS);
				}
				else {
					switch_frame = random_int_on_range(MIN_BLINK_FRAMES, MAX_BLINK_FRAMES);
				}
				blink = !blink;
				redraw = true;
			}
			usleep(5000);
		}
		if(face_1_pid != 0) {
			kill(face_1_pid, SIGINT);
		}
		if(face_2_pid != 0) {
			kill(face_2_pid, SIGINT);
		}
	}
	else {
		exec_gif_drawer(file);
	}
	exit(0);
}

void draw_neutral() {
//	std::cout << "neutral\n";
	draw_image("Images/Faces/Neutral.png");
}

void draw_cheerful() {
//	std::cout << "cheerful\n";
	draw_image("Images/Faces/Cheerful.png");
}

void draw_starry_eyed() {
//	std::cout << "starry-eyed\n";
	draw_image("Images/Faces/StarryEye.png");
}

void draw_excited() {
//	std::cout << "excited\n";
	draw_image("Images/Faces/Excited.png");
}

void draw_shocked_happy() {
//	std::cout << "shockd_happy\n";
	draw_image("Images/Faces/ShockedHap.png");
}

void draw_shocked_sad() {
//	std::cout << "shocked_sad\n";
	draw_image("Images/Faces/ShockedSad.png");
}

void draw_teary() {
//	std::cout << "teary\n";
	draw_image("Images/Faces/Teary.png");
}

void draw_bluescreen() {
//	std::cout << "bluescreen\n";
	draw_image("Images/Bluescreen.png", false);
}

void draw_unimpressed() {
//	std::cout << "unimpressed\n";
	draw_image("Images/Faces/Unimpressed.png");
}

void draw_angry() {
//	std::cout << "angry\n";
	draw_image("Images/Faces/Angry.png");
}

void draw_dead() {
//	std::cout << "dead\n";
	draw_image("Images/Faces/Dead.png");
}

void draw_table_flip() {
//	std::cout << "table flip\n";
	draw_image("Images/Faces/Tableflip.png", false);
}

void draw_party_parrot() {
//	std::cout << "party parrot\n";
	exec_gif_drawer("Images/party_parrot");
}

void draw_rick_roll() {
//	std::cout << "rick roll\n";
	exec_gif_drawer("Images/rick_roll");
}

void draw_the_game() {
//	std::cout << "the game\n";
	draw_image("Images/Faces/TheGame.png", false);
}

void draw_this_is_fine() {
//	std::cout << "this is fine\n";
	draw_image("Images/this_is_fine.png", false);
}

void draw_loading() {
//	std::cout << "loading...\n";
	exec_gif_drawer("Images/loading_screen.gif");
}

void advance_frame_count() {
	curr_frame++;
	if(curr_frame == MAX_FRAME_COUNT) {
		curr_frame = 0;
	}
	if(curr_frame % 100 == 0) {
//		std::cout << "curr_frame = " << curr_frame << ".\n";
	}
}

void setup_next_blink() {
	int num_frames = random_int_on_range(MIN_FRAMES_BETWEEN_BLINKS, MAX_FRAMES_BETWEEN_BLINKS); 
	next_blink_frame = curr_frame + num_frames;
//	std::cout << "Next blink in " << num_frames << ".\n";
	if(next_blink_frame > MAX_FRAME_COUNT) {
		next_blink_frame -= MAX_FRAME_COUNT;
	}
}

void handle_fans() {
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = catch_int_child;
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);

	wiringPiSetup();
	pinMode(6, OUTPUT);
	float temp = 20.f;
	float humidity = 60.f;
	float temp_max = 30.f;
	float humidity_max = 60.f;
	bool fan_is_on = false;
	while(!interrupted) {
		std::ifstream file_in("temp_humidity.txt");
		std::ifstream file_in_max("heat_hum_limits.txt");
		if(file_in.is_open()) {
			std::string readInfo;
			std::getline(file_in, readInfo);
			if(readInfo.size() != 0) {
				temp = std::stof(readInfo);
			}
			std::getline(file_in, readInfo);
			if(readInfo.size() != 0) {
				humidity = std::stof(readInfo);
			}
			file_in.close();
		}
		if(file_in_max.is_open()) {
			std::string readInfo;
			std::getline(file_in_max, readInfo);
			if(readInfo.size() != 0) {
				temp_max = std::stof(readInfo);
			}
			std::getline(file_in_max, readInfo);
			if(readInfo.size() != 0) {
				humidity_max = std::stof(readInfo);
			}
		}
		bool fan_should_be_on = (temp > temp_max || humidity > humidity_max/* || temp > 35.f || humidity > 70.f*/);
		if(fan_should_be_on != fan_is_on) {
			fan_is_on = fan_should_be_on;
			if(fan_is_on) {
				digitalWrite(6, HIGH);
				std::cout << "Turning fan on.\n";
			}
			else {
				digitalWrite(6, LOW);
				std::cout << "Turning fan off.\n";
			}
		}
		sleep(1);
	}
	digitalWrite(6, LOW);
	exit(0);
}

void init_fan() {
	process[FAN] = fork();
	if(process[FAN] == 0) {
		handle_fans();
	}
}

void setup_curr_blink() {
	blink_frames_left = random_int_on_range(MIN_BLINK_FRAMES, MAX_BLINK_FRAMES);
}

bool read_map() {
	std::ifstream file_in("map.txt");
	std::string write_val = "";
	bool ret_val = false;
	if(file_in.is_open()) {
		for(int i = 0; i < 15; i++) {
			std::string readInfo;
			std::getline(file_in, readInfo);
			int index;
			int mapping;
			if(readInfo.size() != 0) {
				index = stoi(readInfo);
				ret_val = true;
			}
			else {
				index = i;
			}
			std::getline(file_in, readInfo);
			if(readInfo.size() != 0) {
				mapping = stoi(readInfo);
			}
			else {
				mapping = map[index];
			}
			map[index] = mapping;
		}
		file_in.close();
	}
	if(!ret_val) {
		return false;
	}
	for(int i = 0; i < 15; i++) {
		write_val += std::to_string(map[i]);
		write_val += "%";
	}
	std::ofstream file_out;
	file_out.open("map.txt", std::ofstream::out | std::ofstream::trunc);
	file_out.close();
	if(ret_val) {
		file_out.open("settings.txt", std::ofstream::out | std::ofstream::trunc);
		file_out << write_val;
		file_out.close();
	}
	return true;
}

// Reads all input files
// Updates the button arrays
// Reads the temperature and humidity sensor
// If the drawing needs to be updated, return true
bool read_files() {
	bool temp_buttons[10];
	for(int i = 0; i < 10; i++) {
		temp_buttons[i] = buttons_real[i];
	}
	int blue_1 = 0;
	wait_to_read();
	std::ifstream file_in("blue_input_1.txt");
	if(file_in.is_open()) {
		std::string readInfo;
		std::getline(file_in, readInfo);
		file_in.close();
		if(readInfo.size() == 0) {
			blue_1 = 100;
		}
		else {
			blue_1 = stoi(readInfo);
		}
	}
	std::ofstream file_out;
	wait_to_read();
	file_out.open("blue_input_1.txt", std::ofstream::out | std::ofstream::trunc);
	file_out.close();
	if(blue_1 != 100) {
		if(blue_1 >= 16) {
			buttons_real[4] = true;
			blue_1 -= 16;
		}
		else {
			buttons_real[4] = false;
		}
		if(blue_1 >= 8) {
			buttons_real[3] = true;
			blue_1 -= 8;
		}
		else {
			buttons_real[3] = false;
		}
		if(blue_1 >= 4) {
			buttons_real[2] = true;
			blue_1 -= 4;
		}
		else {
			buttons_real[2] = false;
		}
		if(blue_1 >= 2) {
			buttons_real[1] = true;
			blue_1 -= 2;
		}
		else {
			buttons_real[1] = false;
		}
		buttons_real[0] = (blue_1 == 1);
	}

	// Mod buttons
	int blue_2 = 0;
	wait_to_read();
	std::string debug_string = "";
	file_in.open("blue_input_2.txt");
	if(file_in.is_open()) {
		std::string readInfo;
		std::getline(file_in, readInfo);
		file_in.close();
		if(readInfo.size() == 0) {
			blue_2 = 100;
		}
		else {
			blue_2 = stoi(readInfo) / 2;
			debug_string = "blue_2 = " + std::to_string(blue_2);
		}
		std::ofstream file_out;
		wait_to_read();
		file_out.open("blue_input_2.txt", std::ofstream::out | std::ofstream::trunc);
		file_out.close();
	}
	if(blue_2 != 100) {
		if(blue_2 >= 16) {
			buttons_real[9] = true;
			blue_2 -= 16;
		}
		else {
			buttons_real[9] = false;
		}
		if(blue_2 >= 8) {
			buttons_real[8] = true;
			blue_2 -= 8;
		}
		else {
			buttons_real[8] = false;
		}
		if(blue_2 >= 4) {
			buttons_real[7] = true;
			blue_2 -= 4;
		}
		else {
			buttons_real[7] = false;
		}
		if(blue_2 >= 2) {
			buttons_real[6] = true;
			blue_2 -= 2;
		}
		else {
			buttons_real[6] = false;
		}
		buttons_real[5] = (blue_2 == 1);
	}

	bool ret_val = read_map();
	for(int i = 0; i < 5; i++) {
		if(buttons_real[i] && !temp_buttons[i]) {
			ret_val = true;
			for(int j = 0; j < 5; j++) {
				buttons_virtual[j] = buttons_real[j];
			}
		}
	}
	if((buttons_real[5] && !temp_buttons[5]) || (buttons_real[6] && !temp_buttons[6])) {
		buttons_virtual[5] = buttons_real[5];
		buttons_virtual[6] = buttons_real[6];
		ret_val = true;
	}
	if(buttons_real[7] == true && !temp_buttons[7]) {
		ret_val = true;
		buttons_virtual[5] = buttons_real[5];
		buttons_virtual[6] = buttons_real[6];
		buttons_virtual[7] = buttons_real[7];
	}
	if(ret_val) {
		std::cout << debug_string << "\n";
	}
	return ret_val;
}

// Stores the drawing functions
void (*function_pointer[])() = {draw_neutral, draw_cheerful, draw_starry_eyed, draw_excited, draw_shocked_happy, draw_shocked_sad, draw_teary, draw_bluescreen, draw_unimpressed, draw_angry, draw_dead, draw_table_flip, draw_party_parrot, draw_rick_roll, draw_the_game, draw_this_is_fine, draw_loading};

// Using the buttons_virtual array and the map array, returns the index
// of the drawing function in the function_pointer array
int get_drawing() {
	int ret = 0;
	if(buttons_virtual[0]) {
		ret = 0;
	}
	if(buttons_virtual[1]) {
		ret = 4;
	}
	if(buttons_virtual[2]) {
		ret = 8;
	}
	if(buttons_virtual[3]) {
		ret = 12;
	}

	if(buttons_virtual[5]) {
		ret++;
	}
	if(buttons_virtual[6]) {
		ret += 2;
	}
	return map[ret];
}

// Saves the map array and temperature/humidity settings for the fan
// TODO: Once fan is installed and bluetooth connection with app is
// set up, figure out a quickly-readable way to save this data
void save_settings() {
	std::ofstream file_out("settings.txt");
	for(int i = 0; i < 16; i++) {
		file_out << i << "%";
	}
}

// Creates a child process which draws the desired face
// Stores the pid of the child process
void draw(void (*func)()) {
	if(process[DRAWING] != 0) {
		kill(process[DRAWING], SIGINT);
	}
	pid_t child_pid = process[DRAWING] = fork();
	if(child_pid == 0) {
		func();
	}
}

// Reads settings.txt to find values to fill the map array and
// the temperature/humidity settings
void load_settings() {
	// Checks to see if settings.txt exists
	std::string filename = "settings.txt";
	std::ifstream file_in(filename.c_str());
	if(file_in.is_open()) {
	std::string readInfo;
		getline(file_in, readInfo);
		std::string temp = readInfo;
		for(int i = 0; i < 15; i++) {
			temp = readInfo.substr(0, readInfo.find("%"));
			readInfo = readInfo.substr(readInfo.find("%")+1);
			map[i] = stoi(temp);
		}

		file_in.close();
	}
	else {
		for(int i = 0; i < 15; i++) {
			map[i] = i;
		}
		save_settings();
	}
}

void debug_check_settings() {
	for(int i = 0; i < 15; i++) {
		std::cout << "map[" << i << "] = " << map[i] << ".\n";
	}
}

// Draws a loading screen for a few seconds, then draws the neutral face
void debug_setup() {
	init_fan();
	// LED Matrix setup
	for(int i = 0; i < NUM_PROCESSES; i++) {
		process[i] = 0;
	}
	color_red = color_green = color_blue = 255;

	// Set up blinking variables
	srand(time(NULL));
	curr_frame = 0;
	blink_frames_left = 0;
	next_blink_frame = 100;
	is_blinking = false;

	// Set up interrupt handler
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = catch_int;
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);
	
//	draw(draw_loading);
//	kill(process[DRAWING], SIGKILL);
//	draw(draw_neutral);
	load_settings();
	debug_check_settings();
	usleep(500);
}

bool debugflag = true;


int debug_counter = 14;
void debug_loop() {
	// Advance frame counter and check if the drawing
	// should blink
	is_blinking = false;
	bool map_updated = read_map();
	if(map_updated || debugflag) {
		draw((*function_pointer[map[0]]));
		debugflag = false;
	}
//	draw((*function_pointer[debug_counter]));
//	std::cout << "debug counter: " << debug_counter << ", map[" << debug_counter << "] = " << map[debug_counter] << "\n";
//	debug_counter++;
//	sleep(3);
//	if(debug_counter > 16) {
//		debug_counter = 0;
//	}
}
	
void debug_main() {
	debug_setup();
	while(true) {
		debug_loop();
//		usleep(20000);
		sleep(5);
	}
}

// Launches the Bluetooth client python scripts and stores their pids
// Will exit if an error occurs in the python scripts
// Draws a loading screen on the matrix during this process
void setup() {
	// Setup process IDs
	for(int i = 0; i < 3; i++) {
		process[i] = 0;
	}
	init_fan();
	
	// Initialize color to white (255, 255, 255)
	color_red   = 255;
	color_green = 255;
	color_blue  = 255;

	// Show the user the process is loading
	draw(draw_loading);

	//Set up the first bluetooth receiver and store its PID
	int pipefd[2];
	pipe(pipefd);
	pid_t ret = process[RECEIVER_1] = fork();
	if(ret == 0) {
		// Child opens the python script
		// Child calls fork, child's child checks for success/failure of python script
			
		// Delete the contents of the file to prevent misreading it
		std::ofstream file_out;
		file_out.open("blue_input_1.txt", std::ofstream::out | std::ofstream::trunc);
		file_out.close();
		pid_t checker_pid = fork();
		if(checker_pid != 0) {
			// Child of main process, opens the python script
			std::string py3 = "python3";
			std::string btr = "bluetooth_receiver_1.py";
			char py3_c[py3.size()+1];
			char btr_c[btr.size()+1];
			strcpy(py3_c, py3.c_str());
			strcpy(btr_c, btr.c_str());
			char * args[] = {py3_c, btr_c, NULL};
			execvp(args[0], args);
			std::cerr << "Failure to open receiver 1\n";
			exit(-1);
		}
		// Now in grandchild process, this reads the output file to see if the receiver works.
		// This is sent to the main process via pipe
		std::string readInfo;
		while(true) {
			wait_to_read();
			std::ifstream file_in("blue_input_1.txt");
			std::getline(file_in, readInfo);
			file_in.close();
			if(readInfo.size() != 0) {
				int read_int = stoi(readInfo);
				if(read_int != -1 && read_int != 100) {
					std::cerr << "failure to check if receiver 1 works.\nExpected 100 or -1, received " << read_int << ".\n";
				}
				close(pipefd[0]);
				write(pipefd[1], &read_int, sizeof(read_int));
				close(pipefd[1]);
				exit(0);
			}	
		}
	}

	// Back in main process, wait for grandchild process to inform of
	// success or failure to set up Bluetooth receiver
	close(pipefd[1]);
	int success = 0;
	read(pipefd[0], &success, sizeof(success));
	close(pipefd[0]);
	if(success == 100) {
//		std::cout << "Successfully connected!\n";
	}
	else {
		std::cerr << "Error: Unable to connect to bluetooth server 1\n";
		exit(-1);
	}

	// Set up the second bluetooth receiver and store its PID
	pipe(pipefd);
	ret = process[RECEIVER_2] = fork();
	if(ret == 0) {
		std::ofstream file_out;
		file_out.open("blue_input_2.txt", std::ofstream::out | std::ofstream::trunc);
		file_out.close();
		pid_t checker_pid = fork();
		if(checker_pid != 0) {
			// Child of main process, executes second Bluetooth receiver script
			std::string py3 = "python3";
			std::string btr = "bluetooth_receiver_2.py";
			char py3_c[py3.size()+1];
			char btr_c[btr.size()+1];
			strcpy(py3_c, py3.c_str());
			strcpy(btr_c, btr.c_str());
			char * args[] = {py3_c, btr_c, NULL};
			execvp(args[0], args);
			std::cerr << "failure to open receiver 2\n";
			exit(0);
		}

		// Grandchild of main process, reads output file to find success
		// or failure to set up Bluetooth receiver, pipes this to main process
		std::string readInfo;
		while(true) {
			wait_to_read();
			std::ifstream file_in("blue_input_2.txt");
			std::getline(file_in, readInfo);
			file_in.close();
			if(readInfo.size() != 0) {
				int read_int = stoi(readInfo);
				if(read_int != -1 && read_int != 200) {
					std::cerr << "failure to chekc if receiver 2 works.\nExpected 200 or -1, received " << read_int << ".\n";
				}
				close(pipefd[0]);
				write(pipefd[1], &read_int, sizeof(read_int));
				close(pipefd[1]);
				exit(0);
			}
		}
	}

	// Back in main process, wait for grandchild process to send
	// success or failure of child process to set up Bluetooth receiver
	close(pipefd[1]);
	success = 0;
	read(pipefd[0], &success, sizeof(success));
	close(pipefd[0]);
	if(success == 200) {
//		std::cout << "Successfully connected!\n";
	}
	else {
		std::cerr << "Error: Unable to connect to bluetooth server 2\n";
		exit(-1);
	}

	// Bluetooth receivers successfully set up, now set up main process.
	for(int i = 0; i < 10; i++) {
		buttons_virtual[i] = false;
		buttons_real[i] = false;
	}
	for(int i = 0; i < 15; i++) {
		map[i] = i;
	}
	load_settings();

	// Set up blinking variables
	srand(time(NULL));
	curr_frame = 0;
	blink_frames_left = 0;
	next_blink_frame = 100;
	is_blinking = false;

	// Set up interrupt handler
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = catch_int;
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);

	draw((*function_pointer[get_drawing()]));
}

// For debugging purposes, outputs the buttons arrays and return
// of the buttons_to_int function
void debug_buttons() {
	std::cout << "real: ";
	for(int i = 0; i < 10; i++) {
		if(buttons_real[i]) {
			std::cout << "1";
		}
		else {
			std::cout << "0";
		}
	}
	std::cout << "\nvirtual: ";
	for(int i = 0; i < 10; i++) {
		if(buttons_virtual[i]) {
			std::cout << "1";
		}
		else {
			std::cout << "0";
		}
	}
	std::cout << "\n";
	std::cout << "Drawing number: " << get_drawing() << std::endl;
}

// Calls for the files to be read and the drawing function if needed.
void loop() {
	// Wait until files are guarunteed to be closed
	wait_to_read();
	
	// Read the files, and if this causes the
	// drawing to need updated, update it
	if(read_files()) {
		draw((*function_pointer[get_drawing()]));
		debug_buttons();
	}
}


// Calls setup, then calls loop infinitely after sleeping 20ms
int main(int argc, char** argv) {
	my_argc = argc;
	my_argv = argv;
//	debug_main();
	setup();
	while(true) {
		usleep(20000);
		loop();
	}
	return 0;
}

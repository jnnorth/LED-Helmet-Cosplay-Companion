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
pid_t process[3];
bool isDrawingAnimation = false;
bool isChildDrawer = false;
volatile bool interrupted = false;

int curr_frame;
int blink_frames_left;
int next_blink_frame;
bool is_blinking;

int my_argc;
char ** my_argv;

// Interrupt handler, closes drawing process and receivers
void catch_int(int sig_num) {
//	std::cout << "I have been killed (main) \n";
	if(isChildDrawer) {
		exit(0);
	}
	kill(process[0], sig_num);
	kill(process[1], sig_num);
	kill(process[2], sig_num);
	
	exit(0);
}

void catch_int_child(int sig_num) {
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
	std::string img_viewer   = "utils/led-image-viewer";
	std::string hardware_pulse_flag = "--led-no-hardware-pulse";
	char img_viewer_c[img_viewer.size()+1];
	char file_c[file.size()+1];
	char pulse_c[hardware_pulse_flag.size()+1];
	strcpy(img_viewer_c, img_viewer.c_str());
	strcpy(file_c, file.c_str());
	strcpy(pulse_c, hardware_pulse_flag.c_str());
	char * args[] = {
		//sudo_c, 
		img_viewer_c, file_c, pulse_c, NULL};
//	execvp(args[1], args);
	execvp(args[0], args);
}


void draw_image(std::string file) {
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = catch_int_child;
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);
	std::string file2 = file.substr(0, file.size()-4);
	file2 += "_blink.png";

	RGBMatrix::Options defaults;
	defaults.pixel_mapper_config = "U-mapper";
	defaults.rows = 32;
	defaults.cols = 128;
	defaults.chain_length = 2;
	defaults.parallel = 1;
	RGBMatrix * myMatrix = rgb_matrix::CreateMatrixFromFlags(&my_argc, &my_argv, &defaults);
	rgb_matrix::FrameCanvas * offscreen_canvas = myMatrix->CreateFrameCanvas();

	int frame = 0;
	int switch_frame = random_int_on_range(MIN_FRAMES_BETWEEN_BLINKS,MAX_FRAMES_BETWEEN_BLINKS);
	bool blink = false;
	bool redraw = false;
	try {
		myMatrix->Clear();
		InitializeMagick(*my_argv);
		Image img1(file);
		Image img2(file2);
		Image img = img1;
		int num_cols = 128;
		int num_rows = 32;
		while(!interrupted) {
			if(redraw) {
				redraw = false;
				for(int i = 0; i < num_cols; i++) {
					for(int j = 0; j < num_rows; j++) {
						if(!blink) {
							img = img1;
						}
						else {
							img = img2;
						}
						ColorRGB rgb(img.pixelColor(i,j));
						int img_red = ScaleQuantumToChar(rgb.redQuantum());
						int img_green = ScaleQuantumToChar(rgb.greenQuantum());
						int img_blue = ScaleQuantumToChar(rgb.blueQuantum());
						img_red *= ((float)(color_red) / 255.f);
						img_green *= ((float)(color_green) / 255.f);
						img_blue *= ((float)(color_blue) / 255.f);
						offscreen_canvas->SetPixel(i, j, img_red, img_green, img_blue);
					}
				}
				offscreen_canvas = myMatrix->SwapOnVSync(offscreen_canvas);
			}
			frame++;
			if(switch_frame == frame) {
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
		delete myMatrix;
		exit(0);
	}
	catch (Magick::Exception & error) {
		std::cerr << "Caught Magick++ exception: " << error.what() << std::endl;
	}
}

void draw_neutral() {
	isDrawingAnimation = false;
	std::cout << "neutral\n";
	draw_image("Images/neutral.png");
}

void draw_cheerful() {
	isDrawingAnimation = false;
	std::cout << "cheerful\n";
//	draw_image("Images/cheerful.png");
}

void draw_starry_eyed() {
	isDrawingAnimation = false;
	std::cout << "starry-eyed\n";
	draw_image("Images/starry.png");
}

void draw_excited() {
	isDrawingAnimation = false;
	std::cout << "excited\n";
	draw_image("Images/excited.png");
}

void draw_shocked_happy() {
	isDrawingAnimation = false;
	std::cout << "shockd_happy\n";
	draw_image("Images/shocked_happy.png");
}

void draw_shocked_sad() {
	isDrawingAnimation = false;
	std::cout << "shocked_sad\n";
	draw_image("Imgaes/shocked_sad.png");
}

void draw_teary() {
	isDrawingAnimation = false;
	std::cout << "teary\n";
	draw_image("Imgaes/teary.png");
}

void draw_bluescreen() {
	isDrawingAnimation = false;
	std::cout << "bluescreen\n";
	draw_image("Imgaes/bluescreen.png");
}

void draw_unimpressed() {
	isDrawingAnimation = false;
	std::cout << "unimpressed\n";
//	draw_image("Images/unimpressed.png");
}

void draw_angry() {
	isDrawingAnimation = false;
	std::cout << "angry\n";
	draw_image("Images/angry.png");
}

void draw_dead() {
	isDrawingAnimation = false;
	std::cout << "dead\n";
	draw_image("Images/dead.png");
}

void draw_table_flip() {
	isDrawingAnimation = true; //TODO: Double check if this is really an animation
	std::cout << "table flip\n";
	exec_gif_drawer("Images/table_flip.stream");
}

void draw_party_parrot() {
	isDrawingAnimation = true;
	exec_gif_drawer("Images/party_parrot.stream");
	std::cout << "party parrot\n";
}

void draw_rick_roll() {
	isDrawingAnimation = true;
	exec_gif_drawer("Images/rick_roll.stream");
	std::cout << "rick roll\n";
}

void draw_the_game() {
	isDrawingAnimation = false;
	std::cout << "the game\n";
	draw_image("Images/the_game.png");
}

void draw_this_is_fine() {
	isDrawingAnimation = false;
	std::cout << "this is fine\n";
	draw_image("Images/this_is_fine.png");
}

void draw_loading() {
	struct sigaction sa;
	sigset_t mask_set;
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = catch_int_child;
	sigfillset(&mask_set);
	sigdelset(&mask_set, SIGINT);
	sa.sa_mask = mask_set;
	sigaction(SIGINT, &sa, NULL);
	RGBMatrix::Options defaults;
	defaults.pixel_mapper_config = "U-mapper";
	defaults.rows = 32;
	defaults.cols = 128;
	defaults.chain_length = 2;
	defaults.parallel = 1;
	RGBMatrix * matrix = rgb_matrix::CreateMatrixFromFlags(&my_argc, &my_argv, &defaults);

	float delta_angle = 1.0 / 360.0;
	float angle = 0;
	float radius = 10.0;
	int x = 42;
	int y = 16;
	int x_old = 42;
	int y_old = 16;
	matrix->Fill(10,10,30);
	while(!interrupted) {
		float delta_x = cos(angle*2*M_PI) * radius;
		float delta_y = sin(angle*2*M_PI) * radius;
		x = (int)(32+delta_x);
		y = (int)(16+delta_y);
		if(x != x_old || y != y_old) {
			matrix->SetPixel(x_old, y_old, 10, 10, 30);
			matrix->SetPixel(x_old+64, y_old, 10, 10, 30);
			x_old = x;
			y_old = y;
		}
		matrix->SetPixel(x, y, 255, 255, 255);
		matrix->SetPixel(x+64, y, 255, 255, 255);
		angle += delta_angle;
		if(angle > 360.0) {
			angle = 0.0;
		}
		usleep(5000);
	}
	delete matrix;
	exit(0);
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

void setup_curr_blink() {
	blink_frames_left = random_int_on_range(MIN_BLINK_FRAMES, MAX_BLINK_FRAMES);
}
// Reads all input files
// Updates the button arrays
// Reads the temperature and humidity sensor
// If the drawing needs to be updated, return true
bool read_files() {
	bool temp_buttons[10];
	int temp_map[15];
	for(int i = 0; i < 10; i++) {
		temp_buttons[i] = buttons_real[i];
	}
	for(int i = 0; i < 15; i++) {
		temp_map[i] = map[i];
	}
	int pipefd[2];
	pipe(pipefd);
	pid_t ret = fork();
	if(ret == -1) {
		perror("fork");
		exit(1);
	}
	else if(ret == 0) {
		close(pipefd[0]);
		int ret = 0;
		wait_to_read();
		std::ifstream file_in("blue_input_1.txt");
		if(file_in.is_open()) {
			std::string readInfo;
			std::getline(file_in, readInfo);
			file_in.close();
			if(readInfo.size() == 0) {
				ret = 100;
				write(pipefd[1], &ret, sizeof(ret));
				close(pipefd[1]);
				exit(0);
			}
			ret = stoi(readInfo);
		}
		std::ofstream file_out;
		wait_to_read();
		file_out.open("blue_input_1.txt", std::ofstream::out | std::ofstream::trunc);
		file_out.close();
		write(pipefd[1], &ret, sizeof(ret));
		close(pipefd[1]);
		exit(0);
	}
	else {
		wait(0);
		close(pipefd[1]);
		int readVal;
		read(pipefd[0], &readVal, sizeof(readVal));
		close(pipefd[0]);
		if(readVal != 100) {
			if(readVal >= 16) {
				buttons_real[4] = true;
				readVal -= 16;
			}
			else {
				buttons_real[4] = false;
			}
			if(readVal >= 8) {
				buttons_real[3] = true;
				readVal -= 8;
			}
			else {
				buttons_real[3] = false;
			}
			if(readVal >= 4) {
				buttons_real[2] = true;
				readVal -= 4;
			}
			else {
				buttons_real[2] = false;
			}
			if(readVal >= 2) {
				buttons_real[1] = true;
				readVal -= 2;
			}
			else {
				buttons_real[1] = false;
			}
			buttons_real[0] = (readVal == 1);
		}
	}

	// Mod buttons
	pipe(pipefd);
	ret = fork();
	if(ret == -1) {
		perror("fork");
		exit(1);
	}
	else if(ret == 0) {
		close(pipefd[0]);
		int ret = 0;
		wait_to_read();
		std::ifstream file_in("blue_input_2.txt");
		if(file_in.is_open()) {
			std::string readInfo;
			std::getline(file_in, readInfo);
			file_in.close();
			if(readInfo.size() == 0) {
				ret = 100;
				write(pipefd[1], &ret, sizeof(ret));
				close(pipefd[1]);
				exit(0);
			}
			ret = stoi(readInfo);
		}
		std::ofstream file_out;
		wait_to_read();
		file_out.open("blue_input_2.txt", std::ofstream::out | std::ofstream::trunc);
		file_out.close();
		write(pipefd[1], &ret, sizeof(ret));
		close(pipefd[1]);
		exit(0);
	}
	else {
		wait(0);
		close(pipefd[1]);
		int readVal;
		read(pipefd[0], &readVal, sizeof(readVal));
		close(pipefd[0]);
		if(readVal != 100) {
			if(readVal >= 16) {
				buttons_real[9] = true;
				readVal -= 16;
			}
			else {
				buttons_real[9] = false;
			}
			if(readVal >= 8) {
				buttons_real[8] = true;
				readVal -= 8;
			}
			else {
				buttons_real[8] = false;
			}
			if(readVal >= 4) {
				buttons_real[7] = true;
				readVal -= 4;
			}
			else {
				buttons_real[7] = false;
			}
			if(readVal >= 2) {
				buttons_real[6] = true;
				readVal -= 2;
			}
			else {
				buttons_real[6] = false;
			}
			buttons_real[5] = (readVal == 1);
		}
	}
	//TODO: Temperature sensor reading
	//
	bool ret_val = false;
	for(int i = 0; i < 5; i++) {
		if(buttons_real[i] && !temp_buttons[i]) {
			ret_val = true;
			for(int j = 0; j < 5; j++) {
				buttons_virtual[j] = buttons_real[j];
			}
		}
	}
	for(int i = 5; i < 10; i++) {
		if(buttons_real[i] != temp_buttons[i]) {
			ret_val = true;
			buttons_virtual[i] = buttons_real[i];
		}
	}
	for(int i = 0; i < 15; i++) {
		if(map[i] != temp_map[i]) {
			ret_val = true;
		}
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

// If the user wants to remap the buttons, update the map array
// TODO: Once bluetooth communication is set up between Pi and app,
// add this function
void update_table() {
	// Actually update the table
	//
	//
	pid_t ret = fork();
	if(ret == 0) {
		save_settings();
		exit(0);
	}
}

// Creates a child process which draws the desired face
// Stores the pid of the child process
void draw(void (*func)()) {
	if(process[DRAWING] != 0) {
		//std::cout << "Killing process with pid: " << process[DRAWING] << "\n";
		kill(process[DRAWING], SIGKILL);
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
			readInfo = readInfo.substr(readInfo.find("@")+1);
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

// Draws a loading screen for a few seconds, then draws the neutral face
void debug_setup() {
	isDrawingAnimation = false;
	// LED Matrix setup
	for(int i = 0; i < 3; i++) {
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
	draw(draw_neutral);
	usleep(500);
}

bool debugflag = true;


int debug_counter = 0;
void debug_loop() {
	// Advance frame counter and check if the drawing
	// should blink
	is_blinking = false;
	if(debug_counter == 0) {
		draw(draw_table_flip);
		debug_counter = 1;
	}
	else if(debug_counter == 1) {
		debug_counter = 2;
		draw(draw_party_parrot);
	}
	else {
		draw(draw_rick_roll);
		debug_counter = 0;
	}
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
	isDrawingAnimation = false;
	// Setup process IDs
	for(int i = 0; i < 3; i++) {
		process[i] = 0;
	}
	
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
				if(read_int != -1 && read_int != 1) {
					std::cerr << "failure to check if receiver 1 works.\nExpected 1 or -1, received " << read_int << ".\n";
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
	if(success == 1) {
		std::cout << "Successfully connected!\n";
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
				if(read_int != -1 && read_int != 1) {
					std::cerr << "failure to chekc if receiver 2 works.\nExpected 1 or -1, received " << read_int << ".\n";
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
	if(success == 1) {
		std::cout << "Successfully connected!\n";
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
	//TODO: Once reading settings is set up, uncomment
//	load_settings();

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
	debug_main();
	setup();
	while(true) {
		usleep(20000);
		loop();
	}
	return 0;
}

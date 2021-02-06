#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include <chrono>
#include <fstream>
#include <time.h>

int map[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
bool buttons_real[10];
bool buttons_virtual[10];
bool mouth_open;

pid_t process[3];
#define RECEIVER_1 0
#define	RECEIVER_2 1
#define DRAWING    3

// Interrupt handler, closes drawing process and receivers
void catch_int(int sig_num) {
	for(int i = 0; i < 3; i++) {
		kill(process[i], SIGKILL);
	}
}

void draw_neutral() {
	std::cout << "neutral\n";
}

void draw_cheerful() {
	std::cout << "cheerful\n";
}

void draw_starry_eyed() {
	std::cout << "starry-eyed\n";
}

void draw_excited() {
	std::cout << "excited\n";
}

void draw_shocked_happy() {
	std::cout << "shockd_happy\n";
}

void draw_shocked_sad() {
	std::cout << "shocked_sad\n";
}

void draw_teary() {
	std::cout << "teary\n";
}

void draw_bluescreen() {
	std::cout << "bluescreen\n";
}

void draw_unimpressed() {
	std::cout << "unimpressed\n";
}

void draw_angry() {
	std::cout << "angry\n";
}

void draw_dead() {
	std::cout << "dead\n";
}

void draw_table_flip() {
	std::cout << "table flip\n";
}

void draw_party_parrot() {
	std::cout << "party parrot\n";
}

void draw_rick_roll() {
	std::cout << "rick roll\n";
}

void draw_the_game() {
	std::cout << "the game\n";
}

void draw_this_is_fine() {
	std::cout << "this is fine\n";
}

void draw_loading() {
	std::cout << "loading...\n";
}

bool read_files() { // returns true if update needed
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
		close(pipefd[0]);
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
		std::ifstream file_in("blue_input_2.txt");
		if(file_in.is_open()) {
			std::string readInfo;
			std::getline(file_in, readInfo);
			file_in.close();
			if(readInfo.size() == 0) {
				write(pipefd[1], &ret, sizeof(ret));
				close(pipefd[1]);
				exit(0);
			}
			ret = stoi(readInfo);
		}
		std::ofstream file_out;
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
	//TODO: Temperature sensor reading
	//
	bool ret_val = false;
	for(int i = 0; i < 5; i++) {
		if(buttons_real[i] && !temp_buttons[i]) {
			ret_val = true;
			for(int j = 0; j < 10; j++) {
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

void (*function_pointer[])() = {draw_neutral, draw_cheerful, draw_starry_eyed, draw_excited, draw_shocked_happy, draw_shocked_sad, draw_teary, draw_bluescreen, draw_unimpressed, draw_angry, draw_dead, draw_table_flip, draw_party_parrot, draw_rick_roll, draw_the_game, draw_this_is_fine, draw_loading};

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

	if(buttons_virtual[6]) {
		ret++;
	}
	if(buttons_virtual[7]) {
		ret += 2;
	}
	return map[ret];
}

void save_settings() {
	std::ofstream file_out("settings.txt");
	for(int i = 0; i < 16; i++) {
		file_out << i << "%";
	}
}

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

void fork_and_draw(void (*func)()) {
	pid_t ret = process[DRAWING] = fork();
	// The drawing function can then call exec to open the python script which does the drawing
	if(ret == 0) {
		func();
		exit(0);
	}
	// Parent process must get drawing processes pid via pipe
}

void draw() {
	fork_and_draw((*function_pointer[get_drawing()]));
}

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

		

void setup() {
	for(int i = 0; i < 3; i++) {
		process[i] = 0;
	}
	fork_and_draw(draw_loading);
	pid_t ret = process[RECEIVER_1] = fork();
	if(ret == 0) {
		std::string py3 = "python3";
		std::string btr = "bluetooth_receiver_1.py";
		char py3_c[py3.size()+1];
		char btr_c[btr.size()+1];
		strcpy(py3_c, py3.c_str());
		strcpy(btr_c, btr.c_str());
		char * args[] = {py3_c, btr_c, NULL};
		execvp(args[0], args);
		std::cerr << "Failure to open receiver 1\n";
		exit(0);
	}
	ret = process[RECEIVER_2] = fork();
	if(ret == 0) {
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
	load_settings();
	for(int i = 0; i < 10; i++) {
		buttons_virtual[i] = false;
		buttons_real[i] = false;
	}
	for(int i = 0; i < 15; i++) {
		map[i] = i;
	}

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

int get_ms_time() {
	using namespace std::chrono;
	uint64_t ms_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	return (int)(ms_time % 1000);
}
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
}

void loop() {
	 
	while(get_ms_time() < 30 || get_ms_time() > 70) {
		// Does nothing
	}
	if(read_files()) {
		draw();
	}
	draw(); // For debugging purposes
	//debug_buttons();
}

int main() {
	setup();
	while(true) {
		usleep(20000);
		loop();
	}
	return 0;
}

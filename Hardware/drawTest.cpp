#include "led-matrix.h"
#include "pixel-mapper.h"

#include <Magick++.h>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <fstream>

using namespace Magick;
using rgb_matrix::GPIO;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int main(int argc, char ** argv) {
	RGBMatrix::Options defaults;
	defaults.pixel_mapper_config = "U-mapper";
	defaults.rows = 32;
	defaults.cols = 64;
	defaults.chain_length = 2;
	defaults.parallel = 1;

	int my_red = 1;
	int my_green = 1;
	int my_blue = 1;

	RGBMatrix *matrix = rgb_matrix::CreateMatrixFromFlags(&argc, &argv, &defaults);

	try {
		InitializeMagick(*argv);
		Image img_test("Images/test1.png");
		int num_cols = img_test.columns();
		int num_rows = img_test.rows();
		int input = 0xffffff;

		// Allow the user to enter a hex number
		std::cin.unsetf(std::ios::dec);
		std::cin.unsetf(std::ios::oct);
		std::cin.unsetf(std::ios::hex);
		while(input != -1) {
			// Input a color
			std::cout << "Enter a hex color (Ex: 0x5fbcd3) or -1 to quit: ";
			std::cin >> input;
			if(input == -1) {
				exit(0);
			}

			// Set color modifiers based on user input
			my_blue = (input & 0x0000ff);
			input >>= 8;
			my_green = input & 0x00ff;
			input >>= 8;
			my_red = input;

			// Draw new image using color modifier
			matrix->Clear();
			for(int i = 0; i < num_rows; i++) {
				for(int j = 0; j < num_cols; j++) {
					ColorRGB rgb(img_test.pixelColor(i,j));
					int img_red   = ScaleQuantumToChar(rgb.redQuantum());
					int img_green = ScaleQuantumToChar(rgb.greenQuantum());
					int img_blue  = ScaleQuantumToChar(rgb.blueQuantum());
					img_red   *= ((float)(my_red)   / 255.f);
					img_green *= ((float)(my_green) / 255.f);
					img_blue  *= ((float)(my_blue)  / 255.f);
					matrix->SetPixel(i, j, img_red, img_green, img_blue);
				}
			}
		}
	}
	catch (Magick::Exception & error) {
		std::cerr << "Caught Magixk++ exception: " << error.what() << std::endl;
	}

	matrix->Clear();
	delete matrix;
	return 0;
}

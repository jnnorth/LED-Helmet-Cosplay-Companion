#include "../rpi-rgb-led-matrix/include/led-matrix.h"
#include "../rpi-rgb-led-matrix/include/pixel-mapper.h"

#include <Magick++.h>
#include <iostream>
#include <signal.h>

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
		Image img_t1("Images/smiley.png");
		int nx_t1 = img_t1.columns();
		int ny_t1 = img_t1.rows();
		int input = 0;
		std::cin.unsetf(std::ios::dec);
		std::cin.unsetf(std::ios::oct);
		std::cin.unsetf(std::ios::hex);
		while(input != -1) {
			std::cout << "Enter a hex color (Ex: 0x5fbcd3): ";
			std::cin >> input;
			for(int i = 0; i < ny_t1; i++) {
				for(int j = 0; j < nx_t1; j++) {
					ColorRGB rgb(img_t1.pixelColor(i,j));
					char img_red   = ScaleQuantumToChar(rgb.redQuantum());
					char img_green = ScaleQuantumToChar(rgb.greenQuantum());
					char img_blue  = ScaleQuantumToChar(rgb.blueQuantum());
					img_red   *= (float)(my_red)   / 255.f;
					img_green *= (float)(my_green) / 255.f;
					img_blue  *= (float)(my_blue)  / 255.f;
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

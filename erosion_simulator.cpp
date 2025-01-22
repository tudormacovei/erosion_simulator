// erosion_simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>

#include "lodepng.h"

char int_to_ascii(unsigned char &value) {
	const char lightness_map[66] = "`^\",:;Il!i~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
	return lightness_map[value / 4];
}

int main()
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned int width, height;

	//decode
	unsigned int error = lodepng::decode(image, width, height, "./heightmap_64.png");

	//if there's an error, display it
	if (error) {
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		return -1;
	}

	// Create a 2D array to store pixel values
	unsigned char** pixelValues = new unsigned char* [height];
	for (int i = 0; i < height; ++i) {
		pixelValues[i] = new unsigned char[width];
	}

	// Copy pixel values to the array
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			// 4 bytes per pixel (RGBA), we poll the R byte - and assume a greyscale image
			pixelValues[i][j] = image[(i * height + j) * 4];
			std::cout << int_to_ascii(pixelValues[i][j]) << ' ';
			
			// clear image to ensure that the writing is without any interference from the read
			image[(i * height + j) * 4] = 0;
			image[(i * height + j) * 4 + 1] = 0;
			image[(i * height + j) * 4 + 2] = 0;
			image[(i * height + j) * 4 + 3] = 0;
		}
		std::cout << '\n';
	}

	// write pixel values to PNG vector (RGBA) bytes
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			// 4 bytes per pixel (RGBA), we poll the R byte - and assume a greyscale image
			image[(i * height + j) * 4] = pixelValues[i][j];
			image[(i * height + j) * 4 + 1] = pixelValues[i][j];
			image[(i * height + j) * 4 + 2] = pixelValues[i][j];
			image[(i * height + j) * 4 + 3] = 255; // Alpha byte
		}
	}

	// Save PNG to disk
	// Encode the image
	error = lodepng::encode("out.png", image, width, height);

	// if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

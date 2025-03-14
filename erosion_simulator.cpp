// erosion_simulator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

#include "lodepng.h"

#define RNG_MARGINS 1		// the number of pixels the droplet placement should be distanced from the edges of the image, at minimum

#define ITERATIONS 100000
#define EVAPORATION 0.005f
#define INTENSITY 6
#define S_DR 0.01f
#define S_DF 0.0005f
#define S_TF 0.0001f
#define S_TR 0.01f
#define STARTING_WATER 1.0f
#define FRICTION_COEFF 0.1f
#define GRAVITATIONAL_CONST 9.8f

#define SOFT_BRUSH true

// Gets the tanget at the given point, with padding at the edges by copying the point's height
std::pair<float, float> get_tangent(float** heights, unsigned int width, unsigned int height, std::pair<unsigned int, unsigned int> point)
{
	float bottom = point.first < height - 1 ? heights[point.first + 1][point.second] : heights[point.first][point.second];
	float right = point.second < width - 1 ? heights[point.first][point.second + 1] : heights[point.first][point.second];
	float left = point.second > 0 ? heights[point.first][point.second - 1] : heights[point.first][point.second];
	float top = point.first > 0 ? heights[point.first - 1][point.second] : heights[point.first][point.second];

	return std::make_pair((bottom - top) / 2.0f, (right - left) / 2.0f);
}

void apply_modification(float** heights, unsigned int width, unsigned int height, std::pair<unsigned int, unsigned int> point, float value)
{
	unsigned int x = point.first;
	unsigned int y = point.second;
	float corner_wieght = 0.15f;
	float ortho_weight = 0.3f;

#if SOFT_BRUSH
	if (x + 1 < height)
	{
		if (y > 0)
		{
			heights[x + 1][y - 1] += value * corner_wieght;
		}
		heights[x + 1][y] += value * ortho_weight;
		if (y + 1 < width)
		{
			heights[x + 1][y + 1] += value * corner_wieght;
		}
	}
	if (x > 0)
	{
		if (y > 0)
		{
			heights[x - 1][y - 1] += value * corner_wieght;
		}
		heights[x - 1][y] += value * ortho_weight;
		if (y + 1 < width)
		{
			heights[x - 1][y + 1] += value * corner_wieght;
		}
	}
	if (y > 0)
	{
		heights[x][y - 1] += value * ortho_weight;
	}
	if (y + 1 < width)
	{
		heights[x][y + 1] += value * ortho_weight;
	}
#endif
	heights[x][y] += value;
}

float get_acceleration(float height_diff)
{
	height_diff = height_diff / 5.0f; // reduce height range to (0, 50)
	float accel_friction = GRAVITATIONAL_CONST * (1.0f / std::sqrt(1.0f + height_diff * height_diff)) * FRICTION_COEFF;
	float accel_front = GRAVITATIONAL_CONST * ((height_diff * height_diff) / std::sqrt(1.0f + height_diff * height_diff));
	
	return (accel_front - accel_friction);
}

// Performs one erosion step by simulating the erosion of one 'droplet'
// Modifies: float** heights
void erosion_step(float** heights, unsigned int width, unsigned int height, std::pair<unsigned int, unsigned int> point)
{
	float water_amount = STARTING_WATER;
	float carried_soil = 0.0f;
	float velocity = 0.0f;
	std::pair<float, float> direction {0.0f, 0.0f};
	
	std::uniform_real_distribution random_float(-1.0f, 1.0f);
	std::mt19937 gen(0);

	auto next_point = point;
	while (water_amount > 0.0f)
	{
		// get the tangent at the current point
		direction = get_tangent(heights, width, height, point);

		// if tangent is close to 0, choose random direction
		if (std::abs(direction.first) <= 0.1f && std::abs(direction.second) <= 0.1f)
		{
			direction.first = random_float(gen);
			direction.second = random_float(gen);
		}

		float slope;
		// based on direction, choose next point and compute slope
		if (std::abs(direction.first) > std::abs(direction.second))
		{
			if (direction.first > 0.0f) // the slope is pointing to the north
			{
				next_point.first -= 1;
			}
			else
			{
				next_point.first += 1;
			}
			slope = std::abs(direction.first);
		}
		else
		{
			if (direction.second > 0.0f) // the slope is pointing to the west
			{
				next_point.second -= 1;
			}
			else
			{
				next_point.second += 1;
			}
			slope = std::abs(direction.second);
		}
		if (next_point.first < 0 || next_point.first >= height || next_point.second < 0 || next_point.second >= width)
		{
			return; // the droplet has left the simulation bounds
		}

		// perform erosion or deposition
		float d_r = S_DR * std::pow(INTENSITY, 2.0f);
		float d_f = S_DF * std::pow(slope, 2.0f / 3.0f) * std::pow(velocity, 2.0f / 3.0f);
		float t_r = S_TR * slope * INTENSITY;
		float t_f = S_TF * std::pow(slope, 5.0f / 3.0f) * std::pow(velocity, 5.0f / 3.0f);
		
		float detached_soil = d_r + d_f;
		float transport_capacity = t_r + t_f;
		
		auto height_diff = heights[point.first][point.second] - heights[next_point.first][next_point.second];
		
		// it does not make sense for the next point to be at a higher position than our current point
		if (height_diff < 0.0f)
		{
			float deposited;
			if (carried_soil < -height_diff / 2.8f)
			{
				deposited = carried_soil;
			}
			else
			{
				deposited = -height_diff;
			}
			carried_soil -= deposited;
			apply_modification(heights, width, height, point, deposited * 0.75f);
			heights[point.first][point.second] += deposited * 2.8f * 0.25f;

			velocity = 0.0f;
			// we do NOT update the point location, it could be permanently stuck
		}
		else
		{
			apply_modification(heights, width, height, point, -detached_soil);
			carried_soil += detached_soil;

			float sedimented_soil = std::max(carried_soil - transport_capacity, 0.0f);

			if (sedimented_soil > 0.0f)
			{
				apply_modification(heights, width, height, point, sedimented_soil);
				carried_soil -= sedimented_soil;
			}
			velocity = (velocity < 0.0f) ? 0.0f : velocity; // velocity cannot be negative
			point = next_point;
		}

		water_amount -= EVAPORATION;
	}
}

void erode_image(unsigned char** pixels, unsigned int width, unsigned int height)
{
	float** heights = new float* [height];
	for (int i = 0; i < height; ++i) {
		heights[i] = new float[width];
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			heights[i][j] = (float) pixels[i][j];
		}
	}

	std::random_device rd;
	std::mt19937 gen(0);	// set a constant seed for testing purposes
	std::uniform_int_distribution<unsigned> distrib_width(RNG_MARGINS, width - RNG_MARGINS);
	std::uniform_int_distribution<unsigned> distrib_height(RNG_MARGINS, height - RNG_MARGINS);

	for (long long i = 0; i < ITERATIONS; i++)
	{
		erosion_step(heights, width, height, std::make_pair(distrib_width(gen), distrib_height(gen)));
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			heights[i][j] = std::clamp(heights[i][j], 0.0f, 255.0f); // to ensure type conversion safety
			pixels[i][j] = (unsigned char)heights[i][j];
		}
	}
}

int main(int argc, char **argv)
{
	std::vector<unsigned char> image; //the raw pixels
	unsigned int width, height;

	if (argc != 3)
	{
		std::cout << "Incorrect number of arguments given! Expected 2.";
		return 1;
	}
	char input_file_name[256];
	char output_file_name[256];
	strncpy(input_file_name, argv[1], 255);
	input_file_name[255] = '\0';
	strncpy(output_file_name, argv[2], 255);
	output_file_name[255] = '\0';

	//decode
	unsigned int error = lodepng::decode(image, width, height, input_file_name);

	//if there's an error, display it
	if (error) {
		std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		return -1;
	}

	// Create a 2D array to store pixel values
	unsigned char** pixels = new unsigned char* [height];
	for (int i = 0; i < height; ++i) {
		pixels[i] = new unsigned char[width];
	}

	// Copy pixel values to the array
	for (long long i = 0; i < height; ++i) {
		for (long long j = 0; j < width; ++j) {
			// 4 bytes per pixel (RGBA), we poll the R byte - and assume a greyscale image
			pixels[i][j] = image[(i * height + j) * 4];
			//std::cout << int_to_ascii(pixels[i][j]) << ' ';
			
			// clear image to ensure that the writing is without any interference from the read
			image[(i * height + j) * 4] = 0;
			image[(i * height + j) * 4 + 1] = 0;
			image[(i * height + j) * 4 + 2] = 0;
			image[(i * height + j) * 4 + 3] = 0;
		}
		//std::cout << '\n';
	}

	erode_image(pixels, width, height);

	// write pixel values to PNG vector (RGBA) bytes
	for (long long i = 0; i < height; ++i) {
		for (long long j = 0; j < width; ++j) {
			// 4 bytes per pixel (RGBA), we poll the R byte - and assume a greyscale image
			image[(i * height + j) * 4] = pixels[i][j];
			image[(i * height + j) * 4 + 1] = pixels[i][j];
			image[(i * height + j) * 4 + 2] = pixels[i][j];
			image[(i * height + j) * 4 + 3] = 255; // Alpha byte
		}
	}

	// Save PNG to disk
	// Encode the image
	error = lodepng::encode(output_file_name, image, width, height);

	// if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    return 0;
}
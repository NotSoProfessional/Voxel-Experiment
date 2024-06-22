#pragma once

#include "Block.h"
#include "Utils.h"
#include <PerlinNoise/PerlinNoise.hpp>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Simplex/SimplexNoise.h"
#include <FastNoise/FastNoise.h>
#include <vector>
#include <random>
#include <cmath>

class Chunk
{
public:
	const Utils::vec3 CHUNK_LOCATION;
	const uint8_t CHUNK_SIZE;
	const uint32_t CHUNK_SQUARED;
	const uint32_t CHUNK_CUBED;

	unsigned int VBO = -1, VAO = -1;

	int blockCnt = 0;

	siv::PerlinNoise* PERLIN;
	SimplexNoise* SIMPLEX;

	bool homogeneous = true;

	Chunk(Utils::vec3 location, uint8_t size, const FastNoise::SmartNode<>& generg) : CHUNK_LOCATION(location), CHUNK_SIZE(size),
		CHUNK_SQUARED(size* size), CHUNK_CUBED(size* size* size) {
		std::vector<float> noiseOut(CHUNK_CUBED);
		//auto gener = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/", FastSIMD::Level_AVX2);
		//auto gener = FastNoise::NewFromEncodedNodeTree("GQANAAIAAAAAACBAKQAACtejPQCPwvW+AQQAAAAAAK5H4T8AAAAAAAAAAAAAAADNzMy9AAAAAAAAAAA=", FastSIMD::Level_AVX2);
		
		int x = (CHUNK_LOCATION.x * (int)CHUNK_SIZE);
		int y = (CHUNK_LOCATION.y * (int)CHUNK_SIZE);
		int z = (CHUNK_LOCATION.z * (int)CHUNK_SIZE);
		FastNoise::OutputMinMax test = generg->GenUniformGrid3D(noiseOut.data(), x, y, z, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.005f, 1234);
		

		for (int i = 0; i < CHUNK_SIZE; i++) {
			for (int j = 0; j < CHUNK_SIZE; j++) {
				for (int k = 0; k < CHUNK_SIZE; k++) {
					int index = j * 32 * 32 + i * 32 + k;
					//double split = (std::floor(test.min) + std::ceil(test.max)) / (double)2;
					int val = (noiseOut[index] < 0) ? 1 : 0;

					if (val) blockCnt++;

					if (!blocks.empty()) {
						if (val != blocks[0].ID) homogeneous = false;
					}

					blocks.push_back(Block(val));
				}
			}
		}

		if (homogeneous) {
			homogeneousBlock = blocks[0];

			blocks.clear();
			blocks.shrink_to_fit();
		}
	}

	Chunk(Utils::vec3 location, uint8_t size, siv::PerlinNoise* perlin, SimplexNoise* simplex, 
			const std::vector<float>& fastnoise) : CHUNK_LOCATION(location), CHUNK_SIZE(size),
				CHUNK_SQUARED(size*size), CHUNK_CUBED(size*size*size), PERLIN(perlin), SIMPLEX(simplex) {

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 rng(rd()); // seed the generator
		std::uniform_int_distribution<int> dist(0, 1); // define the range

		//int index = 0;

		for (int i = 0; i < CHUNK_SIZE; i++) {
			for (int j = 0; j < CHUNK_SIZE; j++) {
				for (int k = 0; k < CHUNK_SIZE; k++) {
					double x = (CHUNK_LOCATION.x * (int)CHUNK_SIZE) + k;
					double y = (CHUNK_LOCATION.y * (int)CHUNK_SIZE) + i;
					double z = (CHUNK_LOCATION.z * (int)CHUNK_SIZE) + j;

					//double scaler = 0.001;
					//double scaler = 0.0025;
					double scaler = 0.005;
					//double scaler = 0.01;
					//double scaler = 0.2;

					//double scaler = 0.1;
					//double scaler = 0.005;

					//const double noise = PERLIN->octave2D_01((x * scaler), (z * scaler), 3, 0.25);
					//const double noise = PERLIN->octave3D_01((x * scaler), (z * scaler), (y * scaler), 10, 0.25);

					//const float noise = SimplexNoise::noise((x * scaler), (z * scaler), (y * scaler));
					//const float noise = SimplexNoise::noise((x * scaler), (z * scaler));
					//const int octaves = static_cast<int>(5 + std::log(400.f));
					//const float noise = SIMPLEX.fractal(octaves, x * scaler, y * scaler) + (z * scaler);
					int index = x + (48*32 * (y + (24*32*z)));
					const float noise = fastnoise[index];

					//const float noise = gener->GenSingle3D(x*scaler, y * scaler, z * scaler, 1234);
					index++;
					//int val = (y / 768.f < abs(noise)) ? 1 : 0;
					int val = (std::round(noise)) ? 0 : 1;

					//int val = 1;
					//int val = ((k + j + i) % 2 == 0) ? 1 : 0;
					//int val = dist(rng);
					//if (dist(rng)) val = 1;
					//if (dist(rng)) val = 1;
					if (val) blockCnt++;
					blocks.push_back(Block(val));
				}
			}
		}
		//blocks = std::vector<Block>(CHUNK_CUBED, Block(1));
	}

	Block GetBlock(const uint32_t& blockLoc) {
		if (homogeneous) return homogeneousBlock;

		return blocks[blockLoc];
	}

	Block GetBlock(const Utils::vec3& blockLoc) {
		return GetBlock(ConvertCoords(blockLoc));
	}

	Utils::vec3 GetWorldLoc(Utils::vec3 blockLoc) {
		int x = (CHUNK_LOCATION.x * CHUNK_SIZE) + blockLoc.x;
		int y = (CHUNK_LOCATION.y * CHUNK_SIZE) + blockLoc.y;
		int z = (CHUNK_LOCATION.z * CHUNK_SIZE) + blockLoc.z;
	
		return Utils::vec3(x, y, z);
	}

	uint32_t ConvertCoords(Utils::vec3 coords) {
		return ConvertCoords(coords.x, coords.y, coords.z);
	}

	uint32_t ConvertCoords(uint16_t x, uint16_t y, uint16_t z) {
		return x + (z * CHUNK_SIZE) + (y * CHUNK_SQUARED);
	}

	std::vector<uint32_t>& GetMeshPoints() {
		//if (meshPoints.size() == 0) return nullptr;

		return meshPoints;
	}

	void AddToBuffer() {
		if (VBO == -1) glGenBuffers(1, &VBO);
		if (VAO == -1) glGenVertexArrays(1, &VAO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t) * meshPoints.size(), meshPoints.data(), GL_STATIC_DRAW);

		glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Draw() {
		if (VBO != -1 && VAO != -1) {
			glBindVertexArray(VAO);
			glDrawArrays(GL_POINTS, 0, meshPoints.size());
		}
	}

private:
	Block homogeneousBlock;
	std::vector<Block> blocks;
	std::vector<uint32_t> meshPoints;
};


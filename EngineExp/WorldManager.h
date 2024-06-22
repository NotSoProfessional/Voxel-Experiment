#pragma once

#include "Chunk.h"
#include "Block.h"
#include "Utils.h"

#include <glm/glm.hpp>
#include <PerlinNoise/PerlinNoise.hpp>
#include "Simplex/SimplexNoise.h"
#include <FastNoise/FastNoise.h>
#include <unordered_map>
#include <cmath>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <functional>

class WorldManager
{
public:
	
	const uint8_t CHUNK_SIZE;
	const uint32_t CHUNK_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

	const siv::PerlinNoise::seed_type seed = 123456u;

	siv::PerlinNoise* PERLIN = new siv::PerlinNoise{ seed };

	SimplexNoise* simplex = new SimplexNoise(0.1f / 400.f, 0.5f, 1.99f, 0.5f);

	void generateNoise3D(int startX, int startY, int startZ, int width, int height, int depth, std::vector<float>&);

	//MeshBuilderNew32* nmb;

	int wSize = 12;
	int wHeight = 48;

	WorldManager(uint8_t chunkSize) : CHUNK_SIZE(chunkSize) {
		//double scaler = 0.08;

		//std::cout << PERLIN->octave3D_11((63 * scaler), (0 * 0.03), (0 * scaler), 3, 0.25) << "\n";
		//std::cout << PERLIN->octave3D_11((64 * scaler), (0 * 0.03), (0 * scaler), 3, 0.25) << "\n\n";
		
		//std::cout << "Using " << fnGenerator->GetSIMDLevel() << "\n";
		//std::cout << "Max Level " << FastSIMD::CPUMaxSIMDLevel() << "\n";
		//nmb = new MeshBuilderNew32();
		float noOfChunks = wSize * wSize * wHeight;
		int genChunks = 0;
		//std::vector<float> noiseOutput(wSize * wSize * wHeight * CHUNK_CUBED);
		std::vector<float> noiseOutput;
		//fnGenerator->GenUniformGrid3D(noiseOutput.data(), 0, 0, 0, wSize* CHUNK_SIZE, wHeight* CHUNK_SIZE, wSize* CHUNK_SIZE, 0.005f, 1337);
		int index = 0;

		const int numThreads = std::thread::hardware_concurrency();

		int groupSize = noOfChunks / numThreads;

		std::cout << numThreads << " Group Size: " << groupSize << "\n";

		std::vector<std::thread> threads;

		//genChunkss(0, 48, noiseOutput);
		std::cout << std::endl;
		for (int i = 0; i < noOfChunks; i += groupSize) {
			//threads.emplace_back(&WorldManager::genChunkss, this, (int)i, (int)(i + groupSize), std::ref(noiseOutput));
			threads.emplace_back(&WorldManager::createChunks, this, (int)i, (int)(i + groupSize));
			//std::thread t1(&WorldManager::genChunkss, this, i, i + groupSize, std::ref(noiseOutput));
		}

		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
		}

		std::cout << "Finished generating world!\n";

		/*for (int i = 0; i < wSize; i++) {
			Utils::PrintProgress(genChunks / noOfChunks);

			for (int j = 0; j < wHeight; j++) {
				for (int k = 0; k < wSize; k++) {


					chunks[Utils::vec3(i, j, k)] = (new Chunk(Utils::vec3(i, j, k), CHUNK_SIZE, PERLIN, simplex, noiseOutput));
					index += CHUNK_CUBED;
					genChunks++;

					//std::cout << ((i+1)*(j+1) * (k+1)) / noOfChunks << "\n";
				}
			}
		}*/
	}

	Block GetBlock(Utils::vec3 blockLoc) {
		int chunkX = floor(blockLoc.x / (float)CHUNK_SIZE);
		int chunkY = floor(blockLoc.y / (float)CHUNK_SIZE);
		int chunkZ = floor(blockLoc.z / (float)CHUNK_SIZE);

		int localX = abs(blockLoc.x) % CHUNK_SIZE;
		int localY = abs(blockLoc.y) % CHUNK_SIZE;
		int localZ = abs(blockLoc.z) % CHUNK_SIZE;

		Utils::vec3 chunkLoc = Utils::vec3(chunkX, chunkY, chunkZ);
		Utils::vec3 localBlockLoc = Utils::vec3(localX, localY, localZ);

		auto chunkIt = chunks.find(chunkLoc);

		if (chunkIt != chunks.end()) {
			return chunkIt->second->GetBlock(localBlockLoc);
		}

		return 0;
	}

	Block GetBlock(Utils::vec3 localBlockLoc, Utils::vec3 chunkLoc) {
		Chunk* chunk = GetChunk(chunkLoc);

		if (chunk != nullptr) {
			return chunk->GetBlock(localBlockLoc);
		}

		return 0;
	}

	Block GetBlock(uint32_t localBlockLoc, Utils::vec3 chunkLoc) {
		Chunk* chunk = GetChunk(chunkLoc);

		if (chunk != nullptr) {
			return chunk->GetBlock(localBlockLoc);
		}

		return 0;
	}

	Chunk* GetChunk(Utils::vec3 chunkLoc) {
		auto chunkIt = chunks.find(chunkLoc);

		if (chunkIt != chunks.end()) {
			return chunkIt->second;
		}

		return nullptr;
	}
	std::unordered_map<Utils::vec3, Chunk*> chunks;
	std::vector<Chunk*> chunksVector;

private:
	FastNoise::SmartNode<> gener = FastNoise::NewFromEncodedNodeTree("IAAgABMAexQuPxoAAREAAgAAAAAA4EAQAAAAiEEfABYAAQAAAAsAAwAAAAIAAAADAAAABAAAAAAAAAA/ARQA//8AAAAAAAA/AAAAAD8AAAAAPwAAAAA/ARcAAACAvwAAgD89ChdAUrgeQBMAAACgQAYAAI/CdTwAmpmZPgAAAAAAAClcDz8BFQAK16M8AAAAAAAAAAAEAAAAAACkcB3AAAAAAAAAAAAAAAAASOH6QAAAAAAAAAAAAJqZGb8BGQANAAIAAAAAACBAKQAACtejPQCPwvW+AQQAAAAAAK5H4T8AAAAAAAAAAAAAAADNzMy9AAAAAAAAAAAAzczMPQ==", FastSIMD::Level_AVX2);

	void createChunks(int chunkStart, int chunkEnd) {
		for (int i = chunkStart; i < chunkEnd; i++) {
			int chunkIndex = i;
			int X = i % wSize;                          // Calculate the x coordinate
			int Z = (i / wSize) % wSize;                // Calculate the z coordinate
			int Y = (i / (wSize * wSize)) % wHeight;     // Calculate the y coordinate
			Y -= wHeight/2;
			Chunk* chunk = new Chunk(Utils::vec3(X, Y, Z), CHUNK_SIZE, gener);

			mu.lock();
			chunks[Utils::vec3(X, Y, Z)] = (chunk);
			mu.unlock();
			//std::cout << "Done chunk " << i << std::endl;;
		}

		//std::cout << "Thread done" << std::endl;
	}

	void genChunkss(int chunkStart, int chunkEnd, std::vector<float>& noiseOut) {
		std::vector<float> noistOutputALL(CHUNK_CUBED*(chunkEnd-chunkStart));
		std::vector<float> noistOutput(CHUNK_CUBED);

		for (int i = chunkStart; i < chunkEnd; i++) {
			int chunkIndex = i;
			int X = i % wSize;                          // Calculate the x coordinate
			int Z = (i / wSize) % wSize;                // Calculate the z coordinate
			int Y = (i / (wSize * wSize)) % 24;     // Calculate the y coordinate

			//const auto noisel = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/", FastSIMD::Level_AVX2);
			noise->GenUniformGrid3D(noistOutput.data(), X* CHUNK_SIZE, Y* CHUNK_SIZE, Z* CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.01f, 1234);
			
			//while (mu.try_lock() == false);
			noistOutputALL.insert(noistOutputALL.begin() + ((i-chunkStart) * CHUNK_CUBED), noistOutput.begin(), noistOutput.end());
			std::cout << "Done chunk " << i << " \n";
		}
		mu.lock();
		noiseOut.insert(noiseOut.begin() + (chunkStart*CHUNK_CUBED), noistOutputALL.begin(), noistOutputALL.end());
		mu.unlock();
		std::cout << "Done\n";
	}

	std::mutex mu;
	//std::vector<float> noiseOutput;
	FastNoise::SmartNode<> noise = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/", FastSIMD::Level_AVX2);
};

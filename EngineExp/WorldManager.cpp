#include "WorldManager.h"

void WorldManager::generateNoise3D(int startX, int startY, int startZ, int width, int height, int depth, std::vector<float>& noiseOut) {
	std::vector<float> out(128*128*768);
	//std::vector<float> out(48 * 48 * 24 * CHUNK_SIZE*2);
	//static auto noise = FastNoise::NewFromEncodedNodeTree("EQACAAAAAAAgQBAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/", FastSIMD::Level_AVX2);
	//std::cout << "Thread!\n";
	//noiseOutput.resize(48 * 48 * 24 * CHUNK_SIZE);
	int scale = 12;
	//noise->GenUniformGrid3D(noiseOut.data(), 0, 0, 0, (128 * scale), 768, (128 * scale), 0.01f, 1337);
	noise->GenUniformGrid3D(out.data(), startX, 0, startZ, 128, 768, 128, 0.01f, 1337);

	/*for (int i = startX; i < width; i++) {
		for (int j = 0; j < height; j++) {
			for (int k = startZ; k < depth; k++) {
				int index = i + (48 * 32 * (j + (24 * 32 * k)));

				noiseOutput[index] = noise->GenSingle3D(i, j, k, 1234);
			}
		}
	}*/


	if (mu.try_lock()) {
		//mu.lock();
	std::cout << "Thread gn\n";
		// 
		//std::unique_lock<std::mutex> lock(mu);
	noiseOut.insert(noiseOut.end(), out.begin(), out.end());

		//mu.unlock();
	}
}

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <KHR/khrplatform.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include "Shaders.h"
#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"
#include <unordered_set>
#include <set>
#include <map>
#include <queue>
#include "MeshBuilder.h"

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

static int gl_width = 1280;
static int gl_height = 960;

const std::uint8_t CHUNK_SIZE = 8;

void error_callback(int error, const char* description) {
	//fprintf(stderr, "Error: %s\n", description);
	Log::GLLogErr("GLFW ERROR: code %i msg: %s\n", error, description);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	gl_width = width;
	gl_height = height;
}

GLint init_used_mem_kb = 0;
GLint cur_avail_mem_kb = 0;
GLint total_mem_kb = 0;

void _update_fps_counter(GLFWwindow* window) {
	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.1) {
		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf_s(tmp, "opengl @ fps: %.2f ftime: %.2f", fps, 1000/fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;

		printf("\33[2K\r");
		glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
			&total_mem_kb);
		glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
			&cur_avail_mem_kb);
		printf("Available video memory: %imb,  Used video memory: %imb~", cur_avail_mem_kb / 1000, ((total_mem_kb - cur_avail_mem_kb) - init_used_mem_kb) / 1000);
	}
	frame_count++;
}

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
};

struct Vertexng {
	float x;
	float y;
	float z;
	float u;
	float v;
};

enum class Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	// Add more directions if needed
};


struct mSize {
	std::uint8_t y =1;
	std::uint8_t x =1;
};

struct mLoc {
	std::uint8_t z = 0;
	std::uint8_t y = 0;
	std::uint8_t x = 0;

	/*struct Hash {
		size_t operator()(const mLoc& loc) const {
			// Combine the hash values of individual members
			return std::hash<std::uint8_t>()(loc.z) ^ (std::hash<std::uint8_t>()(loc.y) << 8) ^ (std::hash<std::uint8_t>()(loc.x) << 16);
		}
	};

	struct Equal {
		bool operator()(const mLoc& lhs, const mLoc& rhs) const {
			return lhs.z == rhs.z && lhs.y == rhs.y && lhs.x == rhs.x;
		}
	};

	bool operator<(const mLoc& rhs) const {
		// Custom less-than operator for mLoc
		return std::tie(z, y, x) < std::tie(rhs.z, rhs.y, rhs.x);
	}


	bool operator==(const mLoc& rhs) const {
		// Custom less-than operator for mLoc
		return std::tie(z, y, x) == std::tie(rhs.z, rhs.y, rhs.x);
	}*/
};

enum Faces {
	BACK = 1,
	FRONT = 2,
	UP = 4,
	DOWN = 8,
	LEFT = 16,
	RIGHT = 32
};

__forceinline mSize*** generateFaceMesh(const std::uint8_t edges[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], Faces face, mSize*** meshSize) {
	/*mSize*** meshSize = new mSize * *[CHUNK_SIZE];
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		meshSize[i] = new mSize * [CHUNK_SIZE];
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			meshSize[i][j] = new mSize[CHUNK_SIZE];
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				meshSize[i][j][k] = mSize(1, 1);
			}
		}
	}*/

	// Allocate memory for the third dimension and initialize with mSize(1, 1)
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			meshSize[i][j] = new mSize[CHUNK_SIZE];
			std::fill(meshSize[i][j], meshSize[i][j] + CHUNK_SIZE, mSize(1, 1));
			//std::memset(meshSize[i][j], 0, CHUNK_SIZE * sizeof(mSize));
		}
	}


	/*mSize*** meshSize = new mSize * *[CHUNK_SIZE];
	mSize** meshData = new mSize * [CHUNK_SIZE * CHUNK_SIZE];
	mSize* flatMeshData = new mSize[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];

	for (int i = 0; i < CHUNK_SIZE; ++i) {
		meshSize[i] = meshData + i * CHUNK_SIZE;
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			meshData[i * CHUNK_SIZE + j] = flatMeshData + (i * CHUNK_SIZE + j) * CHUNK_SIZE;
			for (int k = 0; k < CHUNK_SIZE; ++k) {
				meshSize[i][j][k] = mSize(1, 1);
			}
		}
	}*/


	if (face & BACK || face & FRONT) {
		//std::unordered_multiset<mLoc, mLoc::Hash, mLoc::Equal> locs;
		//std::priority_queue<mLoc> locs;
		//std::vector<mLoc> locs;

		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 1; y < CHUNK_SIZE; y++) {
				for (int x = 0; x < CHUNK_SIZE; x++) {
					if (edges[z][y][x] & face && edges[z][y - 1][x] & face) {
						meshSize[z][y][x].y = meshSize[z][y - 1][x].y + 1;
						meshSize[z][y - 1][x] = mSize(0, 0);
						//locs.push_back(mLoc(z, y, x));
						//locs.erase(std::remove(locs.begin(), locs.end(), mLoc(z, y - 1, x)), locs.end());
						//locs.insert(mLoc(z, y, x));
						//locs.erase(mLoc(z, y - 1, x));
					}

					if (!(edges[z][y][x] & face)) meshSize[z][y][x] = mSize(0, 0);

					if (y == 1) {
						if (!(edges[z][0][x] & face)) meshSize[z][0][x] = mSize(0, 0);
					}
				}
			}
		}

		/*for (mLoc loc : locs) {
			if (meshSize[loc.z][loc.y][loc.x].x || meshSize[loc.z][loc.y][loc.x].y) {
				if (!meshSize[loc.z][loc.y][loc.x - 1].x && !meshSize[loc.z][loc.y][loc.x - 1].y) {
					continue;
				}
				else {
					if ((edges[loc.z][loc.y][loc.x] & face) && (edges[loc.z][loc.y - 1][loc.x] & face)) {
						if (meshSize[loc.z][loc.y][loc.x].y == meshSize[loc.z][loc.y][loc.x - 1].y) {
							meshSize[loc.z][loc.y][loc.x].x = meshSize[loc.z][loc.y][loc.x - 1].x + 1;
							meshSize[loc.z][loc.y][loc.x - 1] = mSize(0, 0);
						}
					}
				}
			}
		}*/

		for (int z = 0; z < CHUNK_SIZE; z++) {
			for (int y = 1; y < CHUNK_SIZE; y++) {
				for (int x = 1; x < CHUNK_SIZE; x++) {
					if (meshSize[z][y][x].x || meshSize[z][y][x].y) {
						if (!meshSize[z][y][x - 1].x && !meshSize[z][y][x - 1].y) {
							continue;
						}
						else {
							if ((edges[z][y][x] & face) && (edges[z][y - 1][x] & face)) {
								if (meshSize[z][y][x].y == meshSize[z][y][x - 1].y) {
									meshSize[z][y][x].x = meshSize[z][y][x - 1].x + 1;
									meshSize[z][y][x - 1] = mSize(0, 0);
								}
							}
						}
					}
				}
			}
		}
	}


	if (face & LEFT || face & RIGHT) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 1; y < CHUNK_SIZE; y++) {
				for (int z = 0; z < CHUNK_SIZE; z++) {
					if (edges[z][y][x] & face && edges[z][y - 1][x] & face) {
						meshSize[z][y][x].y = meshSize[z][y - 1][x].y + 1;
						meshSize[z][y - 1][x] = mSize(0, 0);
					}

					if (!(edges[z][y][x] & face)) meshSize[z][y][x] = mSize(0, 0);

					if (y == 1) {
						if (!(edges[z][0][x] & face)) meshSize[z][0][x] = mSize(0, 0);
					}
				}
			}
		}

		for (int x = 0; x < CHUNK_SIZE; x++) {
			for (int y = 1; y < CHUNK_SIZE; y++) {
				for (int z = 1; z < CHUNK_SIZE; z++) {
					if (meshSize[z][y][x].x || meshSize[z][y][x].y) {
						if (!meshSize[z-1][y][x].x && !meshSize[z-1][y][x].y) {
							continue;
						}
						else {

							//edges[z][y][x] &&
							if ((edges[z][y][x] & face) && (edges[z][y - 1][x] & face)) {
								if (meshSize[z][y][x].y == meshSize[z-1][y][x].y) {
									meshSize[z][y][x].x = meshSize[z-1][y][x].x + 1;
									meshSize[z-1][y][x] = mSize(0, 0);
								}
							}
						}
					}
				}
			}
		}
	}

	if (face & UP || face & DOWN) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 1; z < CHUNK_SIZE; z++) {
				for (int x = 0; x < CHUNK_SIZE; x++) {
					if (edges[z][y][x] & face && edges[z-1][y][x] & face) {
						meshSize[z][y][x].y = meshSize[z-1][y][x].y + 1;
						meshSize[z-1][y][x] = mSize(0, 0);
					}

					if (!(edges[z][y][x] & face)) meshSize[z][y][x] = mSize(0, 0);

					if (z == 1) {
						if (!(edges[0][y][x] & face)) meshSize[0][y][x] = mSize(0, 0);
					}
				}
			}
		}

		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int z = 1; z < CHUNK_SIZE; z++) {
				for (int x = 1; x < CHUNK_SIZE; x++) {
					if (meshSize[z][y][x].x || meshSize[z][y][x].y) {
						if (!meshSize[z][y][x - 1].x && !meshSize[z][y][x - 1].y) {
							continue;
						}
						else {

							//edges[z][y][x] &&
							if ((edges[z][y][x] & face) && (edges[z-1][y][x] & face)) {
								if (meshSize[z][y][x].y == meshSize[z][y][x - 1].y) {
									meshSize[z][y][x].x = meshSize[z][y][x - 1].x + 1;
									meshSize[z][y][x - 1] = mSize(0, 0);
								}
							}
						}
					}
				}
			}
		}
	}
	return meshSize;
}

/*for (int z = 0; z < 8; z++) {
	for (int x = 0; x < 8; x++) {
		for (int y = 1; y < 8; y++) {
			if (meshSize[z][y][x].x || meshSize[z][y][x].y) {
				if (!meshSize[z][y][x - 1].x && !meshSize[z][y][x - 1].y) {
					continue;
				}
				else {

					//edges[z][y][x] &&
					if ((edges[z][y][x] & face) && (edges[z][y-1][x] & face)) {
						if (meshSize[z][y][x].y == meshSize[z][y][x - 1].y) {
							meshSize[z][y][x].x = meshSize[z][y][x - 1].x + 1;
							meshSize[z][y][x - 1] = mSize(0, 0);
						}
					}
				}
			}
		}
	}
}*/


__forceinline void generateVertices(mSize*** meshSize, std::vector<Vertexng>& vertices, Faces face, std::vector<int>& indices) {
	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				if (meshSize[z][y][x].x != 0 || meshSize[z][y][x].y != 0) {
					/*float zPos = static_cast<float>(z) + 1;
					float xPos = static_cast<float>(x) +1;
					float yPos = static_cast<float>(y) +1;

					float xSize = static_cast<float>(meshSize[z][y][x].x);
					float ySize = static_cast<float>(meshSize[z][y][x].y);*/

					float zPos = z + 1;
					float xPos = x + 1;
					float yPos = y + 1;

					float xSize = meshSize[z][y][x].x;
					float ySize = meshSize[z][y][x].y;

					Vertexng v1, v2, v3, v4;

					if (face & FRONT || face & BACK) {
						if (face & BACK) {
							zPos -= 1;
						}

						/*v1 = {glm::vec3(xPos, yPos, zPos), glm::vec2(0.0f, 0.0f)};
						v2 = { glm::vec3(xPos - xSize, yPos, zPos), glm::vec2(1.0f * xSize, 0.0f) };
						v3 = { glm::vec3(xPos - xSize, yPos - ySize, zPos), glm::vec2(1.0f * xSize, 1.0f * ySize) };
						v4 = { glm::vec3(xPos, yPos - ySize, zPos), glm::vec2(0.0f, 1.0f * ySize) };*/

						v1 = { xPos, yPos, zPos, 0.0f, 0.0f};
						v2 = { xPos - xSize, yPos, zPos, 1.0f * xSize, 0.0f };
						v3 = { xPos - xSize, yPos - ySize, zPos, 1.0f * xSize, 1.0f * ySize };
						v4 = { xPos, yPos - ySize, zPos, 0.0f, 1.0f * ySize };

					}

					if (face & LEFT || face & RIGHT) {
						if (face & LEFT) {
							xPos -= 1;
						}

						/*v1 = {glm::vec3(xPos, yPos, zPos), glm::vec2(0.0f, 0.0f)};
						v2 = { glm::vec3(xPos, yPos, zPos - xSize), glm::vec2(1.0f * xSize, 0.0f) };
						v3 = { glm::vec3(xPos, yPos - ySize, zPos - xSize), glm::vec2(1.0f * xSize, 1.0f * ySize) };
						v4 = { glm::vec3(xPos, yPos - ySize, zPos), glm::vec2(0.0f, 1.0f * ySize) };*/

						v1 = { xPos, yPos, zPos, 0.0f, 0.0f };
						v2 = { xPos, yPos, zPos - xSize, 1.0f * xSize, 0.0f };
						v3 = { xPos, yPos - ySize, zPos - xSize, 1.0f * xSize, 1.0f * ySize };
						v4 = { xPos, yPos - ySize, zPos, 0.0f, 1.0f * ySize };
					}

					if (face & UP || face & DOWN) {
						if (face & DOWN) {
							yPos -= 1;
						}

						/*v1 = {glm::vec3(xPos, yPos, zPos), glm::vec2(0.0f, 0.0f)};
						v2 = { glm::vec3(xPos - xSize, yPos, zPos), glm::vec2(1.0f * xSize, 0.0f) };
						v3 = { glm::vec3(xPos - xSize, yPos, zPos - ySize), glm::vec2(1.0f * xSize, 1.0f * ySize) };
						v4 = { glm::vec3(xPos, yPos, zPos - ySize), glm::vec2(0.0f, 1.0f * ySize) };*/

						v1 = { xPos, yPos, zPos, 0.0f, 0.0f };
						v2 = { xPos - xSize, yPos, zPos, 1.0f * xSize, 0.0f };
						v3 = { xPos - xSize, yPos, zPos - ySize, 1.0f * xSize, 1.0f * ySize };
						v4 = { xPos, yPos, zPos - ySize, 0.0f, 1.0f * ySize };
					}

					// Add indices for the current quad
					int vertexOffset = vertices.size();

					if (face & DOWN || face & LEFT || face & FRONT) {
						indices.emplace_back(vertexOffset);
						indices.emplace_back(vertexOffset + 1);
						indices.emplace_back(vertexOffset + 3);
						indices.emplace_back(vertexOffset + 2);
						indices.emplace_back(vertexOffset + 3);
						indices.emplace_back(vertexOffset + 1);
					}
					else {
						indices.emplace_back(vertexOffset + 1);
						indices.emplace_back(vertexOffset + 3);
						indices.emplace_back(vertexOffset + 2);
						indices.emplace_back(vertexOffset + 3);
						indices.emplace_back(vertexOffset + 1);
						indices.emplace_back(vertexOffset);
					}

					vertices.emplace_back(v1);
					vertices.emplace_back(v2);
					vertices.emplace_back(v3);
					vertices.emplace_back(v4);
				}
			}
		}
	}
}

__forceinline void generate3DMesh(const std::uint8_t dchunk[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], std::vector<Vertexng>& vertices, std::vector<int>& indices) {

	std::uint8_t edges[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = { 0 };

	for (int z = 0; z < CHUNK_SIZE; z++) {
		for (int y = 0; y < CHUNK_SIZE; y++) {
			for (int x = 0; x < CHUNK_SIZE; x++) {
				if (dchunk[z][y][x]) {
					if (z == 0) edges[z][y][x] |= BACK; // back
					if (z == CHUNK_SIZE-1) edges[z][y][x] |= FRONT; // front

					if (y == CHUNK_SIZE - 1) edges[z][y][x] |= UP; // up
					if (y == 0) edges[z][y][x] |= DOWN; // down

					if (x == 0) edges[z][y][x] |= LEFT; // left
					if (x == CHUNK_SIZE - 1) edges[z][y][x] |= RIGHT; // right

					if (z != 0 && !dchunk[z - 1][y][x]) edges[z][y][x] |= BACK; // back
					if (z != CHUNK_SIZE - 1 && !dchunk[z + 1][y][x]) edges[z][y][x] |= FRONT; // front

					if (y != CHUNK_SIZE - 1 && !dchunk[z][y + 1][x]) edges[z][y][x] |= UP; // up
					if (y != 0 && !dchunk[z][y - 1][x]) edges[z][y][x] |= DOWN; // down

					if (x != 0 && !dchunk[z][y][x - 1]) edges[z][y][x] |= LEFT; // left
					if (x != CHUNK_SIZE - 1 && !dchunk[z][y][x + 1]) edges[z][y][x] |= RIGHT; // right
				}
			}
		}
	}

	mSize*** meshSize = new mSize * *[CHUNK_SIZE];

	// Allocate memory for the second dimension
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		meshSize[i] = new mSize * [CHUNK_SIZE];
	}

	int exSize = 0;

	mSize*** meshSizeBACK = generateFaceMesh(edges, BACK, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeBACK, vertices, BACK, indices);

	mSize*** meshSizeFRONT = generateFaceMesh(edges, FRONT, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeFRONT, vertices, FRONT, indices);

	mSize*** meshSizeUP = generateFaceMesh(edges, UP, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeUP, vertices, UP, indices);

	mSize*** meshSizeDOWN = generateFaceMesh(edges, DOWN, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeDOWN, vertices, DOWN, indices);

	mSize*** meshSizeLEFT = generateFaceMesh(edges, LEFT, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeLEFT, vertices, LEFT, indices);

	mSize*** meshSizeRIGHT = generateFaceMesh(edges, RIGHT, meshSize);
	//vertices.reserve(4);
	generateVertices(meshSizeRIGHT, vertices, RIGHT, indices);

	/*std::cout << edges[0][0][0] << "\n";

	if (edges[0][0][0] & BACK) std::cout << "BACK\n";
	if (edges[0][0][0] & FRONT) std::cout << "FRONT\n";
	if (edges[0][0][0] & UP) std::cout << "UP\n";
	if (edges[0][0][0] & DOWN) std::cout << "DOWN\n";
	if (edges[0][0][0] & LEFT) std::cout << "LEFT\n";
	if (edges[0][0][0] & RIGHT) std::cout << "RIGHT\n";


	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int k = 0; k < 8; k++) {
				std::cout << (edges[i][j][k] & BACK) << ", ";
			}

			std::cout << "\n";
		}

		std::cout << "\n";
	}

	std::cout << "\n";
	std::cout << "\n";
	std::cout << "\n";

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int k = 0; k < 8; k++) {
				std::cout << "(" << meshSizeBACK[i][j][k].y << ", " << meshSizeBACK[i][j][k].x << "), ";
			}

			std::cout << "\n";
		}

		std::cout << "\n";
	}*/
}

void generateMesh(const int chunk[16][16], std::vector<Vertex>& vertices) {
	mSize meshSize[16][16] = {};

	for (int y = 1; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			if (chunk[y][x] && chunk[y-1][x]) {
				meshSize[y][x].y = meshSize[y-1][x].y + 1;
				meshSize[y - 1][x] = mSize(0,0);
			}
#
			if (!chunk[y][x]) meshSize[y][x] = mSize(0, 0);

			if (y == 1) {
				if (!chunk[0][x]) meshSize[0][x] = mSize(0, 0);
			}
		}
	}

	//TODO: USE PAIRS/UNORDERED MAP TO ONLY LOOP THROUGH LOCATIONS WITH A SIZE

	for (int x = 1; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			if (meshSize[y][x].x || meshSize[y][x].y) {
				if (!meshSize[y][x - 1].x && !meshSize[y][x - 1].y) {
					continue;
				}
				else {
					if (chunk[y][x] && chunk[y - 1][x]) {
						if (meshSize[y][x].y == meshSize[y][x - 1].y) {
							meshSize[y][x].x = meshSize[y][x-1].x + 1;
							meshSize[y][x-1] = mSize(0, 0);
						}
					}
				}
			}
		}
	}

	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			if (meshSize[y][x].x != 0 || meshSize[y][x].y != 0) {
				float xPos = static_cast<float>(x);
				float yPos = static_cast<float>(y);

				float xSize = static_cast<float>(meshSize[y][x].x);
				float ySize = static_cast<float>(meshSize[y][x].y);

				Vertex v1 = { glm::vec3(xPos, yPos, 0.0f), glm::vec2(0.0f, 0.0f) };
				Vertex v2 = { glm::vec3(xPos - xSize, yPos, 0.0f), glm::vec2(1.0f*xSize, 0.0f) };
				Vertex v3 = { glm::vec3(xPos - xSize, yPos - ySize, 0.0f), glm::vec2(1.0f*xSize, 1.0f*ySize) };
				Vertex v4 = { glm::vec3(xPos, yPos - ySize, 0.0f), glm::vec2(0.0f, 1.0f*ySize) };

				// Add vertices to the vector
				vertices.push_back(v1);
				vertices.push_back(v2);
				vertices.push_back(v3);

				vertices.push_back(v1);
				vertices.push_back(v3);
				vertices.push_back(v4);
			}
		}
	}

	std::cout << "Finished!\n";
}

void generateFace(const int chunk[16][16], int x, int y, Direction direction, std::vector<Vertex>& vertices) {
	// Generate quad vertices
	float xPos = static_cast<float>(x);
	float yPos = static_cast<float>(y);

	Vertex v1 = { glm::vec3(xPos, yPos, 0.0f), glm::vec2(0.0f, 0.0f) };
	Vertex v2 = { glm::vec3(xPos + 1.0f, yPos, 0.0f), glm::vec2(1.0f, 0.0f) };
	Vertex v3 = { glm::vec3(xPos + 1.0f, yPos + 1.0f, 0.0f), glm::vec2(1.0f, 1.0f) };
	Vertex v4 = { glm::vec3(xPos, yPos + 1.0f, 0.0f), glm::vec2(0.0f, 1.0f) };

	// Add vertices to the vector
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);

	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);
}

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = gl_width / 2.0f;
float lastY = gl_height / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

int main() {
	assert(Log::RestartGLLog());

	Log::GLLog("started GLFW\n%s\n", glfwGetVersionString());

	if (!glfwInit()) {
		//std::cout << "Failed to initialise GLFW!" << std::endl;
		Log::GLLogErr("GLFW ERROR: failed to initialise!");
		return 1;
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(1280, 960, "EngineExp", NULL, NULL);
	if (!window) {
		Log::GLLogErr("GLFW ERROR: window or opengl context creation failed!");
		//std::cout << "Window or OpenGL context creation failed!" << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);
	if (!gladLoadGL()) {
		Log::GLLogErr("GLAD ERROR: failed to initialise function pointers!");
		//std::cout << "Glad failed to initialise function pointers!" << std::endl;
	}

	glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
		&cur_avail_mem_kb);
	glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
		&total_mem_kb);

	init_used_mem_kb = total_mem_kb - cur_avail_mem_kb;

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	Log::GLParams();

	double startTime = glfwGetTime();

	Shaders::LoadAll();

	double endTime = glfwGetTime();

	/*float points[] = {
		0.0f,  0.5f,  0.0f,
		0.5f, -0.5f,  0.0f,
		-0.5f, -0.5f,  0.0f
	};*/

	/*float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};*/

	float vertices[] = {
		// positions          // colors           // texture coords
		
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	-0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};

	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float cube[] = {
		0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//std::cout << "Loaded, compiled, and linked shaders in " << (endTime - startTime)*1000 << " miliseconds" << '\n';
	Log::GLLog("Loaded, compiled, and linked shaders in %f miliseconds\n", (endTime - startTime) * 1000);

	Shaders::ListShaders();
	Shaders::ListPrograms();


	//std::cout << Shaders::GetUniformLoc("projectedProgram", "view") << "\n";

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	int chunk[16][16] = {
		{ 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1 }, 
		{ 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0 }, 
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0 }, 
		{ 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0 }, 
		{ 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0 }, 
		{ 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1 }, 
		{ 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0 }, 
		{ 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1 }, 
		{ 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0 }, 
		{ 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1 }, 
		{ 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0 }, 
		{ 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0 }, 
		{ 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 }, 
		{ 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1 }, 
		{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0 }, 
		{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0 }
	};

	std::cout << (sizeof(chunk) / sizeof(*chunk)) << "\n";


	std::vector<Vertexng> vertices2;

	//generateMesh(chunk, vertices2);

	std::uint8_t dchunk[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	
	MeshBuilder mb = MeshBuilder();
	uint8_t flatchunk[CHUNK_SIZE* CHUNK_SIZE* CHUNK_SIZE];

	int faces = 0;

	for (int i = 0; i < CHUNK_SIZE; i++) {
		for (int j = 0; j < CHUNK_SIZE; j++) {
			for (int k = 0; k < CHUNK_SIZE; k++) {
				//int val = std::rand() % 2;
				//int val = 1;

				int val = ((k + j + i) % 2 == 0) ? 1 : 0;
				//int val = 0;
				//if ((k + j + i) % 3 == 0) val = 1;

				faces += val * 6;
				dchunk[i][j][k] = val;

				flatchunk[mb.ConvertCoords(k, j, i)] = val;
				//dchunk[i][j][k] = ((k+j+i) % 2 == 0) ? 1 : 0;
				//if ((k + j + i) % 3 == 0) dchunk[i][j][k] = 1;
				//dchunk[i][j][k] = 1;
				//std::cout << dchunk[i][j][k] << ", ";
			}

			//std::cout << "\n";
		}

		//std::cout << "\n";
	}


	std::cout << "\n";
	std::cout << faces<<" total faces in chunk\n";
	std::cout << "\n";

	std::vector<int> indices2;

	startTime = glfwGetTime();
	//generate3DMesh(dchunk, vertices2, indices2);
	endTime = glfwGetTime();

	startTime = glfwGetTime();
	std::vector<uint16_t> verts;

	mb.BuildMesh(flatchunk, verts);
	endTime = glfwGetTime();

	std::cout << "Generated chunk mesh in " << (endTime - startTime) * 1000 << "ms\n";
	std::cout << verts.size() << "\n";

	unsigned int eVBO, eVAO;
	glGenVertexArrays(1, &eVAO);
	glGenBuffers(1, &eVBO);

	glBindBuffer(GL_ARRAY_BUFFER, eVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint16_t)*verts.size(), verts.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(eVAO);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_SHORT, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	/*unsigned int VBO2, VAO2, EBO2;
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertexng)* vertices2.size(), vertices2.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices2.size(), indices2.data(), GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	std::cout << (indices2.size() / 3) / 2 << " faces rendered, reduction of " << 100 - (((indices2.size() / static_cast<float>(3)) / 2) / faces) * 100.0f << "%\n";*/


	GLuint axisVAO, axisVBO;
	GLfloat axis[] = {
		// X-axis
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Y-axis
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Z-axis
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &axisVAO);
	glGenBuffers(1, &axisVBO);

	glBindVertexArray(axisVAO);

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis), axis, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	//model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::translate(view, glm::vec3(-8.0f, -5.0f, -20.0f));
	//view = glm::rotate(view, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//view = glm::rotate(view, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK); // or GL_FRONT, depending on your winding order

	float draw_distance = 100.0f;

	GLfloat tgPoint[] = {
		0.0f, 0.0f, 0.0f
	};

	GLuint tgVBO, tgVAO;
	glGenVertexArrays(1, &tgVAO);
	glGenBuffers(1, &tgVBO);

	glBindVertexArray(tgVAO);

	glBindBuffer(GL_ARRAY_BUFFER, tgVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tgPoint), &tgPoint, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		_update_fps_counter(window);

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glViewport(0, 0, gl_width, gl_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/*Shaders::UseProgram("genQuad");

		glClearColor(0.6f, 0.6f, 0.8f, 1.0f);

		projection = glm::perspective(glm::radians(45.0f), (float)gl_width / (float)gl_height, 0.1f, draw_distance);
		glm::mat4 view = camera.GetViewMatrix();

		glUniformMatrix4fv(Shaders::GetUniformLoc("genQuad", "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(Shaders::GetUniformLoc("genQuad", "view"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(Shaders::GetUniformLoc("genQuad", "projection"), 1, GL_FALSE, glm::value_ptr(projection));



		glBindTexture(GL_TEXTURE_2D, texture);


		glBindVertexArray(eVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, verts.size());*/

		Shaders::UseProgram("pQuad");

		projection = glm::perspective(glm::radians(45.0f), (float)gl_width / (float)gl_height, 0.1f, draw_distance);
		glm::mat4 view = camera.GetViewMatrix();
		
		glUniformMatrix4fv(Shaders::GetUniformLoc("pQuad", "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(Shaders::GetUniformLoc("pQuad", "viewMatrix"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(Shaders::GetUniformLoc("pQuad", "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindTexture(GL_TEXTURE_2D, texture);

		glBindVertexArray(eVAO);
		glDrawArrays(GL_POINTS, 0, verts.size());

		glBindVertexArray(0);
		glUseProgram(0);


		glfwPollEvents();

		glfwSwapBuffers(window);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, 1);
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F1)) Shaders::ReloadProgramShaders();
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F2)) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F3)) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F5)) draw_distance--;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F6)) draw_distance++;
		//if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F7)) CHUNK_SIZE = 32;
		//if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F8)) CHUNK_SIZE = 64;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(Camera_Movement::WEST, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(Camera_Movement::EAST, deltaTime);
	}

	glfwTerminate();
	return 0;
}
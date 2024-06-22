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
#include <FastSIMD/FastSIMD.h>
#include "Shaders.h"
#include "Log.h"
#include "Chunk.h"
#include "WorldManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Camera.h"
#include <unordered_map>
#include <locale>
#include "MeshBuilder.h"
#include "MeshBuilderNew8.h"
#include "MeshBuilderNew16.h"
#include "MeshBuilderNew32.h"
#include <thread>

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

static int gl_width = 1280;
static int gl_height = 960;

MeshBuilder* mb;

enum ChunkType {
	ALL,
	EVERY_OTHER,
	RANDOM
};

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

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
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

void CreateMesh(int startChunk, int endChunk, WorldManager& world) {
	MeshBuilder* mb = new MeshBuilderNew32();

	for (int i = startChunk; i < endChunk; i++) {
		int chunkIndex = i;
		int X = i % world.wSize;                          // Calculate the x coordinate
		int Z = (i / world.wSize) % world.wSize;                // Calculate the z coordinate
		int Y = (i / (world.wSize * world.wSize)) % world.wHeight;     // Calculate the y coordinate
		Y -= world.wHeight / 2;

		Chunk* chunk = world.chunks[Utils::vec3(X, Y, Z)];

		mb->BuildMesh(chunk, &world, chunk->GetMeshPoints());
		//chunk->AddToBuffer();
		//std::cout << "Done chunk " << i << std::endl;;
	}

	delete mb;
}

int main() {
	std::setlocale(LC_ALL, "en_US.UTF-8");

	if (FastSIMD::CPUMaxSIMDLevel() & FastSIMD::Level_AVX2 & FastSIMD::CPUMaxSIMDLevel() & FastNoise::SUPPORTED_SIMD_LEVELS & FastSIMD::COMPILED_SIMD_LEVELS) {
		std::cout << "AVX2 is supported!" << std::endl;
	}
	else {
		std::cout << "AVX2 is NOT supported!" << std::endl;
	}

	int input = -1;

	std::cout << "Select chunk size:\n";
	std::cout << "1) 8x8x8\n";
	std::cout << "2) 16x16x16\n";
	std::cout << "3) 32x32x32\n\n";

	WorldManager* world = nullptr;

	while (input == -1) {
		std::cin >> input;

		switch (input) {
		case 1:
			//mb = new MeshBuilderNew8();
			break;

		case 2:
			mb = new MeshBuilderNew16();
			world = new WorldManager(16);
			break;

		case 3:
			std::cout << "Generating chunks...\n\n";
			mb = new MeshBuilderNew32();
			world = new WorldManager(32);
			break;

		default:
			std::cout << "Invalid input...\n\n";
			std::cin.clear();
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			input = -1;

			break;
		}
	}


	ChunkType chunkType;

	std::cout << "Select chunk type:\n";
	std::cout << "1) All (Best Case)\n";
	std::cout << "2) Every Other (Worst Case)\n";
	std::cout << "3) Random\n\n";

	input = -1;

	while (input == -1) {
		std::cin >> input;

		switch (input) {
		case 1:
			chunkType = ALL;
			break;

		case 2:
			chunkType = EVERY_OTHER;
			break;

		case 3:
			chunkType = RANDOM;
			break;

		default:
			std::cout << "Invalid input...\n\n";
			std::cin.clear();
			//std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			input = -1;

			break;
		}
	}

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

	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

	int outputMonitor = 0;

	if (monitorCount > 1) outputMonitor = 1;

	const GLFWvidmode* mode = glfwGetVideoMode(monitors[outputMonitor]);

	int xPos, yPos;
	glfwGetMonitorPos(monitors[outputMonitor], &xPos, &yPos);

	GLFWwindow* window = glfwCreateWindow(gl_width, gl_height, "EngineExp", NULL, NULL);
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

	glfwSetWindowPos(window, ((mode->width - gl_width)/2) + xPos, ((mode->height - gl_height)/2) + yPos);

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

	//std::cout << "Loaded, compiled, and linked shaders in " << (endTime - startTime)*1000 << " miliseconds" << '\n';
	Log::GLLog("Loaded, compiled, and linked shaders in %f miliseconds\n", (endTime - startTime) * 1000);

	//Shaders::ListShaders();
	//Shaders::ListPrograms();

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
	

	std::cout << "\n";
	std::cout << "\n";

	Chunk* chunk = nullptr;
	double sizen = 0;
	int vb = 0;
	int bCnt = 0;
	int faceCnt = 0;
	int pointCnt = 0;
	//std::vector<uint32_t>* points;
	//std::srand(std::time(nullptr));

	std::cout << "Generating meshes...\n\n";
	std::vector<Utils::vec3> chunkLocsToRemove;
	std::cout << world->chunks.size() << "\n";

	int chunksProcessed = 0;
	float totalChunks = world->chunks.size();

	const int numThreads = std::thread::hardware_concurrency();

	int groupSize = totalChunks / numThreads;

	std::cout << numThreads << " Group Size: " << groupSize << "\n";

	std::vector<std::thread> threads;

	//genChunkss(0, 48, noiseOutput);
	std::cout << std::endl;
	for (int i = 0; i < totalChunks; i += groupSize) {
		threads.emplace_back(&CreateMesh, (int)i, (int)(i + groupSize), std::ref(*world));
	}

	for (int i = 0; i < threads.size(); i++) {
		threads[i].join();
	}

	/*for (const auto chunkIt : world->chunks) {
		if (chunkIt.second != nullptr) {
			chunk = chunkIt.second;

			startTime = glfwGetTime();
			mb->BuildMesh(chunk, world, chunk->GetMeshPoints());

			endTime = glfwGetTime();
			chunk->AddToBuffer();
			vb += mb->VisibleBlocks;
			bCnt += chunk->blockCnt;
			pointCnt += chunk->GetMeshPoints().size();

			if (chunk->GetMeshPoints().size() == 0) {
				//world->chunks.erase(chunk->CHUNK_LOCATION);
				chunkLocsToRemove.push_back(chunk->CHUNK_LOCATION);
			}

			sizen += (4 * chunk->GetMeshPoints().size()) / 1024.f / 1024.f;

			chunksProcessed++;
			if (chunksProcessed % 250 == 0) Utils::PrintProgress(chunksProcessed / totalChunks);
		}
	}*/

	std::cout << "Removing empty meshes...\n\n";



	//world->chunks.erase_if()
	/*for (const auto chunkIt : world->chunks) {
		if (chunkIt.second != nullptr) {
			std::cout << chunk->CHUNK_LOCATION.x << ", " << chunk->CHUNK_LOCATION.y << ", " << chunk->CHUNK_LOCATION.z << "\n";
			std::cout << chunk->GetMeshPoints().size() << "\n\n";

			if (chunk->GetMeshPoints().size() == 0) {
				chunkLocsToRemove.push_back(chunk->CHUNK_LOCATION);
			}
		}
	}*/

	for (const auto loc : chunkLocsToRemove) {
		world->chunks.erase(loc);
	}

	std::cout << world->chunks.size() << "\n";

	for (const auto chunkIt : world->chunks) {
		Chunk* chunk = chunkIt.second;
		if (chunk->GetMeshPoints().size() > 0) {
			chunk->AddToBuffer();
			world->chunksVector.emplace_back(chunk);
		}
	}

	std::cout << world->chunksVector.size() << "\n";

	faceCnt = bCnt * 6;
	double reduction = 100 - (((float)pointCnt / faceCnt) * 100.f);

	std::cout << "Generated chunk mesh in " << (endTime - startTime) * 1000 << "ms\n";
	std::cout << "Visible blocks " << vb << "\n";
	std::cout << "No. of points " << pointCnt << "\n";
	std::cout << "Reduction from culling and meshing " << reduction << "%\n";
	std::cout << "Chunk size in buffer: " << sizen << "MB\n";
	std::cout << "Average chunk size in buffer: " << sizen/world->chunks.size()*1024 << "KB\n\n";


	std::vector<uint8_t> flatchunk(mb->CHUNK_CUBED);

	int faces = 0;

	std::srand(std::time(nullptr));

	for (int i = 0; i < mb->CHUNK_SIZE; i++) {
		for (int j = 0; j < mb->CHUNK_SIZE; j++) {
			for (int k = 0; k < mb->CHUNK_SIZE; k++) {

				int val;

				switch (chunkType) {
				case ALL:
					val = 1;
					break;

				case EVERY_OTHER:
					val = ((k + j + i) % 2 == 0) ? 1 : 0;

					break;

				case RANDOM:
					val = std::rand() % 2;
					//if (std::rand() % 2) val = 1;
					break;
				}
			
				// For chunk type of most unique blocks possible
				//if (!i || !j || !k) val = 1;
				//if (i==7 || j==7 || k==7) val = 1;
				 
				// For chunk type for every third block
				//int val = ((k + j + i) % 3 == 0) ? 1 : 0;

				faces += val * 6;

				flatchunk[mb->ConvertCoords(k, j, i)] = val;

				//std::cout << mb16.ConvertCoords(k, j, i) << std::endl;
			}

			//std::cout << "\n";
		}

		//std::cout << "\n";
	}


	std::cout << "\n";
	std::cout << "\n";
	
	startTime = glfwGetTime();

	std::vector<uint32_t> verts;
	mb->BuildMesh(flatchunk.data(), verts);
	
	endTime = glfwGetTime();

	reduction = 100 - (((float) verts.size() / faces) * 100.f);
	double size = (4 * verts.size()) / 1024.f;

	std::cout << "Generated chunk mesh in " << (endTime - startTime) * 1000 << "ms\n";
	std::cout << "Visible blocks " << mb->VisibleBlocks << "\n";
	std::cout << "No. faces in chunk " << faces << "\n";
	std::cout << "No. of points " << verts.size() << "\n";

	//std::setprecision(1);
	std::cout << "Reduction from culling and meshing " << reduction << "%\n";
	std::cout << "Chunk size in buffer: " << size << "KB\n\n";


	unsigned int eVBO, eVAO;
	glGenVertexArrays(1, &eVAO);
	glGenBuffers(1, &eVBO);

	//glBindBuffer(GL_ARRAY_BUFFER, eVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t)*vertsn.size(), vertsn.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(eVAO);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Old Axis object
	/*GLuint axisVAO, axisVBO;
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
	glBindVertexArray(0);*/

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	float draw_distance = 600.0f;

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


	const char* chunkShader = mb->SHADER->c_str();

	GLuint chunkProgram = Shaders::GetProgramId(chunkShader);

	GLint modelLoc = Shaders::GetUniformLoc(chunkShader, "modelMatrix");
	GLint viewLoc = Shaders::GetUniformLoc(chunkShader, "viewMatrix");
	GLint projectionLoc = Shaders::GetUniformLoc(chunkShader, "projectionMatrix");

	GLint chunkLocationLoc = Shaders::GetUniformLoc(chunkShader, "chunkLocation");

	uint32_t nbOfChunks = 2;
	glm::vec3 chunkLocation(0);

	//std::cout << "" << typeid(*mb).name() << "\n";
	//std::cout << "" << typeid(MeshBuilderNew16).name() << "\n";

	bool useVector = true;


	std::vector<float> chunkLocations;
	for (const auto& chunk : world->chunksVector) {
		chunkLocations.push_back(chunk->CHUNK_LOCATION.x);
		chunkLocations.push_back(chunk->CHUNK_LOCATION.y);
		chunkLocations.push_back(chunk->CHUNK_LOCATION.z);
	}

	GLuint chunkLocationBuffer;
	glGenBuffers(1, &chunkLocationBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, chunkLocationBuffer);
	glBufferData(GL_ARRAY_BUFFER, chunkLocations.size() * sizeof(float), chunkLocations.data(), GL_STATIC_DRAW);

	while (!glfwWindowShouldClose(window)) {
		_update_fps_counter(window);

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glViewport(0, 0, gl_width, gl_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		Shaders::UseProgram(chunkProgram);

		projection = glm::perspective(glm::radians(45.0f), (float)gl_width / (float)gl_height, 0.1f, draw_distance);
		glm::mat4 view = camera.GetViewMatrix();
		
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindTexture(GL_TEXTURE_2D, texture);

		//glBindVertexArray(eVAO);

		/*if (typeid(*mb) == typeid(MeshBuilderNew32) || typeid(*mb) == typeid(MeshBuilderNew16)) {
			for (int x = 0; x < nbOfChunks; x++) {
				for (int y = 0; y < 24; y++) {
					for (int z = 0; z < nbOfChunks; z++) {
						chunkLocation.x = x;
						chunkLocation.y = y;
						chunkLocation.z = z;

						//std::cout << chunkLocation.x << " " << chunkLocation.y << " " << chunkLocation.z << std::endl;
						glUniform3fv(chunkLocationLoc, 1, glm::value_ptr(chunkLocation));

						glDrawArrays(GL_POINTS, 0, verts.size());
					}
				}
			}
		}
		else {
			glDrawArrays(GL_POINTS, 0, verts.size());
		}*/

		/*for (auto chunk : world->chunks) {
			chunkLocation.x = chunk.first.x;
			chunkLocation.y = chunk.first.y;
			chunkLocation.z = chunk.first.z;

			glUniform3fv(chunkLocationLoc, 1, glm::value_ptr(chunkLocation));

			glDrawArrays(GL_POINTS, 0, verts.size());
		}*/
		if (!useVector) {
			for (const auto chunkIt : world->chunks) {
				if (chunkIt.second != nullptr) {
					Chunk* chunk = chunkIt.second;
					//std::vector<uint32_t> points = chunk->GetMeshPoints();
					//float coords[] = { chunk->CHUNK_LOCATION.x,chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z };
					//float coords[] = { 0.f,0.f, 0.f };
					//chunkLocation.x = chunk->CHUNK_LOCATION.x;
					//chunkLocation.y = chunk->CHUNK_LOCATION.y;
					//chunkLocation.z = chunk->CHUNK_LOCATION.z;
						//glBindVertexArray(chunk->VAO);

					glUniform3fv(chunkLocationLoc, 1, chunk->CHUNK_LOCATION.coords);
					chunk->Draw();
				}
			}
		}
		else {
			for (const auto chunk : world->chunksVector) {
					float coords[] = { chunk->CHUNK_LOCATION.x,chunk->CHUNK_LOCATION.y, chunk->CHUNK_LOCATION.z };

					glUniform3fv(chunkLocationLoc, 1, coords);
					chunk->Draw();
			}
		}

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
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F7)) glEnable(GL_CULL_FACE);
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F8)) glDisable(GL_CULL_FACE);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F9)) useVector = false;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F10)) useVector = true;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			deltaTime += .08f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			deltaTime += .32f;

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
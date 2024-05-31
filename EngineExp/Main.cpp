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
#include "MeshBuilder8.h"
#include "MeshBuilder16.h"
#include "MeshBuilder.h"
#include "MeshBuilderNew8.h"
#include "MeshBuilderNew16.h"

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

int main() {
	
	int input = -1;

	std::cout << "Select chunk size:\n";
	std::cout << "1) 8x8x8\n";
	std::cout << "2) 16x16x16\n\n";

	while (input == -1) {
		std::cin >> input;

		switch (input) {
		case 1:
			mb = new MeshBuilderNew8();
			break;

		case 2:
			mb = new MeshBuilderNew16();
			break;

		default:
			std::cout << "Invalid input...\n\n";
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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

	Shaders::ListShaders();
	Shaders::ListPrograms();

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
	

	//MeshBuilder* mb = new MeshBuilderNew8();

	std::vector<uint8_t> flatchunk(mb->CHUNK_CUBED);

	int faces = 0;

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
					break;
				}

			
				//if (!i || !j || !k) val = 1;
				//if (i==7 || j==7 || k==7) val = 1;
				 
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
	std::cout << faces<<" total faces in chunk\n";
	std::cout << "\n";
	
	startTime = glfwGetTime();

	std::vector<uint32_t> verts;
	mb->BuildMesh(flatchunk.data(), verts);
	
	endTime = glfwGetTime();

	std::cout << "Generated chunk mesh in " << (endTime - startTime) * 1000 << "ms\n";
	std::cout << "Visible blocks " << mb->VisibleBlocks << "\n";
	std::cout << verts.size() << "\n";


	unsigned int eVBO, eVAO;
	glGenVertexArrays(1, &eVAO);
	glGenBuffers(1, &eVBO);

	glBindBuffer(GL_ARRAY_BUFFER, eVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint32_t)*verts.size(), verts.data(), GL_STATIC_DRAW);
	
	glBindVertexArray(eVAO);
	glVertexAttribIPointer(0, 1, GL_UNSIGNED_INT, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


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
	//view = glm::translate(view, glm::vec3(-8.0f, -5.0f, -20.0f));
	//view = glm::rotate(view, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//view = glm::rotate(view, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

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


	const char* chunkShader = mb->SHADER->c_str();

	GLuint chunkProgram = Shaders::GetProgramId(chunkShader);

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
		
		glUniformMatrix4fv(Shaders::GetUniformLoc(chunkShader, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(Shaders::GetUniformLoc(chunkShader, "viewMatrix"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(Shaders::GetUniformLoc(chunkShader, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));

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
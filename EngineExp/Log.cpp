#include "Log.h"
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdarg>
#include <memory>
#include <glad/glad.h>

bool Log::RestartGLLog() {
	std::ofstream file(GL_LOG_FILE);

	if (!file.is_open()) {
		std::cerr << "ERROR: could not open GL_LOG_FILE log file " << GL_LOG_FILE << " for writing\n";
		return false;
	}

	time_t now = time(NULL);
	char date[26];

	if (ctime_s(date, sizeof(date), &now) != 0) {
		std::cerr << "ERROR: could not get local time\n";
		return false;
	}

	file << "GL_LOG_FILE log. local time " << date << std::endl;

	return true;
}

void Log::GLParams() {
	GLenum params[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO,
	};
	const char* names[] = {
	  "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
	  "GL_MAX_DRAW_BUFFERS",
	  "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
	  "GL_MAX_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_TEXTURE_SIZE",
	  "GL_MAX_VARYING_FLOATS",
	  "GL_MAX_VERTEX_ATTRIBS",
	  "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
	  "GL_MAX_VIEWPORT_DIMS",
	  "GL_STEREO",
	};
	Log::GLLog("GL Context Params:\n");
	char msg[256];
	// integers - only works if the order is 0-10 integer return types
	for (int i = 0; i < 10; i++) {
		int v = 0;
		glGetIntegerv(params[i], &v);
		Log::GLLog("%s %i\n", names[i], v);
	}
	// others
	int v[2];
	v[0] = v[1] = 0;
	glGetIntegerv(params[10], v);
	Log::GLLog("%s %i %i\n", names[10], v[0], v[1]);
	unsigned char s = 0;
	glGetBooleanv(params[11], &s);
	Log::GLLog("%s %u\n", names[11], (unsigned int)s);
	Log::GLLog("-----------------------------\n");
}

bool Log::GLLog(const char* message, ...) {
	std::ofstream file(GL_LOG_FILE, std::ios_base::app);

	if (!file.is_open()) {
		std::cerr << "ERROR: could not open GL_LOG_FILE " << GL_LOG_FILE << " file for appending\n";
		return false;
	}

	va_list argptr;
	va_start(argptr, message);

	char buffer[256];
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	file << buffer;

	va_end(argptr);

	return true;
}

bool Log::GLLogErr(const char* message, ...) {
	std::ofstream file(GL_LOG_FILE, std::ios_base::app);

	if (!file.is_open()) {
		std::cerr << "ERROR: could not open GL_LOG_FILE " << GL_LOG_FILE << " file for appending\n";
		return false;
	}

	va_list argptr;
	va_start(argptr, message);

	char buffer[256];
	vsnprintf(buffer, sizeof(buffer), message, argptr);
	file << buffer;

	va_end(argptr);

	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	va_end(argptr);

	return true;
}
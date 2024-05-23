#pragma once

#include <time.h>
#include <stdarg.h>
#define GL_LOG_FILE "gl.log"

class Log
{
public:
	static bool RestartGLLog();
	static void GLParams();
	static bool GLLog(const char* message, ...);
	static bool GLLogErr(const char* message, ...);
};


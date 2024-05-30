#pragma once

#include "glad/glad.h"
#include <unordered_map>
#include <string>
#include <filesystem>

class Shaders {
public:
    static void LoadAll();
    static void UseProgram(std::string);
    static void UseProgram(char*);
    static void UseProgram(GLuint);
    static void ReloadProgramShaders();

    static GLint GetUniformLoc(const GLchar*, const GLchar*);
    static GLint GetUniformLoc(GLuint, const GLchar*);
    static GLuint GetProgramId(std::string);

    static void ListPrograms();
    static void ListShaders();

private:
    struct Shader {
        GLuint id;
        std::string name;
        std::filesystem::path sourcePath;
        std::vector<GLuint> programs;
        std::unordered_map<std::string, GLint> uniformLocations;
    };

    static void CreateShaders();
    static void LoadPrograms();
    static void CompileShaderFromPath(GLuint id, std::filesystem::path path);

    static std::unordered_map<std::string, GLuint> programs;
    static std::vector<Shader*> shaders;
    static std::unordered_map<GLuint, std::vector<Shaders::Shader*>> programShadersIndex;
    static std::unordered_map<std::string, GLint> uniformLocations;
};
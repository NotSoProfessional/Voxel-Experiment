#include "Shaders.h"
#include <iostream>
#include <fstream>

#include "Log.h"

std::unordered_map<std::string, GLuint> Shaders::programs;
std::unordered_map<GLuint, std::vector<Shaders::Shader*>> Shaders::programShadersIndex;
std::vector<Shaders::Shader*> Shaders::shaders;
std::unordered_map<std::string, GLint> Shaders::uniformLocations;

void Shaders::LoadAll() {
    Log::GLLog("creating all shaders and programs\n");

    CreateShaders();
    LoadPrograms();
}

void Shaders::CreateShaders() {
    Log::GLLog("creating shaders\n");

    const std::unordered_map<std::string, int> SHADER_TYPE = {
        {"vertex", GL_VERTEX_SHADER},
        {"fragment", GL_FRAGMENT_SHADER},
        {"geometry", GL_GEOMETRY_SHADER}
    };

    std::string directory_path = "shaders/";

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory_path)) {
            //std::cout << entry.path() << "\n";

            std::string subDir = entry.path().stem().string();

            if (entry.is_directory()) {
                for (const auto& entry : std::filesystem::directory_iterator(entry.path())) {
                    if (entry.path().extension() == ".glsl") {
                        //std::cout << "Found shader " << entry.path().filename() << "\n";
                        Log::GLLog("found shader %s\n", entry.path().filename().string().c_str());

                        auto it = SHADER_TYPE.find(subDir);

                        if (it != SHADER_TYPE.end()) {
                            GLuint shaderID = glCreateShader(it->second);

                            CompileShaderFromPath(shaderID, entry.path());

                            Shader *shader = new Shader;
                            shader->id = shaderID;
                            shader->name = entry.path().stem().string();
                            shader->sourcePath = entry.path();

                            shaders.push_back(shader);
                        }
                        else {
                            //std::cout << "ERROR: Failed to create shader for " << entry.path().filename()
                            //    << " as it's shader type, \"" << subDir << "\" is unknown." << "\n";

                            Log::GLLogErr("SHADER ERROR: failed to create shader for %s as its shader type, \"%s\" is unknown\n",
                                entry.path().filename().string().c_str(), subDir.c_str());
                        }
                    }
                }
            }
        }
    }
    catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error accessing directory: " << ex.what() << std::endl;
    }
}

void Shaders::LoadPrograms() {
    Log::GLLog("loading programs\n");

    std::string line;
    std::ifstream file("shaders/programs.txt");

    if (file.is_open()) {
        std::string programName;
        GLuint program = -1;

        while (getline(file, line)) {
            if (line == "#") {
                if (program != -1) {
                    glLinkProgram(program);
                }

                getline(file, line);
                programName = line;
                program = glCreateProgram();
                programs[programName] = program;
            }
            else {
                auto it = std::find_if(shaders.begin(), shaders.end(), [line](const Shader *shader) {
                    return shader->name == line;
                    });

                if (it != shaders.end()) {
                    Shaders::Shader *shader = *it;

                    glAttachShader(program, shader->id);
                    shader->programs.push_back(program);
                    programShadersIndex[program].push_back(shader);
                }
                else {
                    //std::cout << "ERROR: Could not find shader \"" << line << "\" for program \"" << programName << "\"\n";
                    Log::GLLogErr("SHADER/PROGRAM ERROR: could not find shader \"%s\" for program \"%s\"\n", line.c_str(), programName.c_str());
                }
            }
        }

        glLinkProgram(program);

        int success;
        char infoLog[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            std::cerr << "Error: Shader program linking failed\n" << infoLog << std::endl;
            // Handle the error appropriately, e.g., return an error code or throw an exception
        }


        for (int i = 0; i < shaders.size(); i++) {
            if (shaders[i]->programs.empty()) {
                glDeleteShader(shaders[i]->id);

                delete shaders[i];

                shaders.erase(shaders.begin() + i);
            }
        }

        file.close();
    }
    else {
        //std::cout << "ERROR: Failed to open programs.txt" << "\n";
        Log::GLLogErr("PROGRAM ERROR: failed to open programs.txt\n");
    }
}

void Shaders::ReloadProgramShaders() {
    Log::GLLog("reloading program shaders\n");

    for (auto [name, id] : programs) {
        for (const Shader *shader : programShadersIndex.at(id)) {
            CompileShaderFromPath(shader->id, shader->sourcePath);
        }

        glLinkProgram(id);
    }

    //glReleaseShaderCompiler();
}

void Shaders::CompileShaderFromPath(GLuint id, std::filesystem::path path) {
    Log::GLLog("compiling shader from path: %s with id %i\n", path.string().c_str(), id);

    std::ifstream shaderFile(path);
    if (shaderFile.is_open()) {
        std::string shaderContents((std::istreambuf_iterator<char>(shaderFile)),
            std::istreambuf_iterator<char>());

        const GLchar* shaderContentsCS = shaderContents.c_str();

        glShaderSource(id, 1, &shaderContentsCS, NULL);
        glCompileShader(id);

        int success;
        char infoLog[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(id, 512, nullptr, infoLog);
            std::cerr << "Error: Shader compilation failed: " << path.stem() << "\n" << infoLog << std::endl;
            // Handle the error appropriately, e.g., return an error code or throw an exception
        }

        shaderFile.close();
    }
    else {
        //std::cout << "ERROR: Could not open " << path << "\n";
        Log::GLLogErr("SHADER ERROR: could not open %s\n", path.string().c_str());
    }
}

void Shaders::UseProgram(std::string pName) {
    //Log::GLLog("using program \"%s\"\n", pName.c_str());

    auto it = programs.find(pName);

    if (it != programs.end()) {
        glUseProgram(it->second);
    }
    else {
        //std::cout << "ERROR: Could not find program \"" << pName << "\"\n";
        Log::GLLogErr("PROGRAM ERROR: could not find program \"%s\"\n", pName);
    }
}

GLint Shaders::GetUniformLoc(const GLchar* name, const GLchar* uniform) {
    const auto it = programs.find(name);

    return (it != programs.end()) ? GetUniformLoc(it->second, uniform) : -1;
}

GLint Shaders::GetUniformLoc(GLuint id, const GLchar* uniform) {
    std::string sID = uniform;
    sID += id;

    const auto it = uniformLocations.find(sID);

    if (it != uniformLocations.end()) {
        return it->second;
    }
    else {
        GLint location = glGetUniformLocation(id, uniform);

        uniformLocations[sID] = location;

        return location;
    }
}

void Shaders::ListPrograms() {
    for (auto program : programs) {
        std::cout << program.first << "  |   " << program.second << "\n";
    }
}

void Shaders::ListShaders() {
    for (auto shader : shaders) {
        std::cout << shader->name << std::setw(6) << "|" << std::setw(6) << shader->id << "\n";
    }
}
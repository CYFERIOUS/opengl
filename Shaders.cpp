#include <iostream>
#include <fstream>
#include <sstream>
#include "Shaders.h"



    Shaders::Shaders(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexShaderSource = readShadersFile(vertexPath);
        std::string fragmentShaderSource = readShadersFile(fragmentPath);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const GLchar* sourceVertex = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &sourceVertex, nullptr);
        glCompileShader(vertexShader);
        printShadersLog(vertexShader, "VERTEX");

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const GLchar* sourceFragment = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &sourceFragment, nullptr);
        glCompileShader(fragmentShader);
        printShadersLog(fragmentShader, "FRAGMENT");

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        printProgramLog(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        int nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;
        
    }

    GLuint Shaders::getProgram() {
        return shaderProgram;
    }

    void Shaders::use(GLuint ID)
    {
        glUseProgram(ID);
    }


    std::string Shaders::readShadersFile(const std::string& filePath) {
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            std::cerr << "ERROR: Failed to open shader file: " << filePath << std::endl;
            return "";
        }

        std::stringstream sstr;
        sstr << fileStream.rdbuf();
        fileStream.close();

        return sstr.str();
    }

    void Shaders::printShadersLog(GLuint shader, const char* name) {
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "ERROR: Shader compile failed (" << name << ")\n" << infoLog << std::endl;
        }
    }

    void Shaders::printProgramLog(GLuint program) {
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
            std::cerr << "ERROR: Program link failed\n" << infoLog << std::endl;
        }
    }

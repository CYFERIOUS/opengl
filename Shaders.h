#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shaders
{
public:
    // the program ID
    GLuint shaderProgram;

    // constructor reads and builds the shader
    Shaders(const std::string& vertexPath, const std::string& fragmentPath);
	// use/activate the shader
    GLuint getProgram();
    void use(GLuint ID);
private:
	// utility function for checking shader compilation/linking errors.
    std::string readShadersFile(const std::string& filePath);
	// utility function for logging shader compilation errors
    static void printShadersLog(GLuint shader, const char* name);
	// utility function for logging program linking errors
    static void printProgramLog(GLuint program);
    // use/activate the shader
	
};

#endif
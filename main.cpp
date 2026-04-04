#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include "Shaders.h"
#include "Origin.h"



 struct VertexTriangle {
	GLfloat pos[3];
	GLfloat color[4]; // RGBA
	GLfloat angle;
};

 using Vertex3angle = VertexTriangle;

 static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
 {
	 // Update GL viewport to new framebuffer size
	 glViewport(0, 0, width, height);

 }


int main() {
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	

	Vertex3angle triangles [] = {

		// Triangle 0 (blue, opaque) // 0 deg in radians
		{ { -0.3f, -0.3f, 0.3f }, { 0.0f, 0.0f, 1.0f, 1.0f }, 0.0f },
		{ {  0.3f, -0.3f, 0.3f }, { 0.0f, 0.0f, 1.0f, 1.0f }, 0.0f },
		{ {  0.0f,  0.3f, 0.3f }, { 0.0f, 0.0f, 1.0f, 1.0f }, 0.0f },

		// Triangle 1 (red, rotated 30 degrees) // 30 deg in radians
		{ { -0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 0.4f },  0.0f },
		{ {  0.5f, -0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 0.4f }, 0.0f },
		{ {  0.0f,  0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f, 0.4f }, 0.0f },

		 // Triangle 2 (green, semi-transparent) // 60 deg in radians
		{ { -0.8f, -0.8f, 0.8f }, { 0.0f, 1.0f, 0.0f, 0.6f }, 0.0f },
		{ {  0.8f, -0.8f, 0.8f }, { 0.0f, 1.0f, 0.0f, 0.6f }, 0.0f },
		{ {  0.0f,  0.8f, 0.8f }, { 0.0f, 1.0f, 0.0f, 0.6f }, 0.0f }

		
	};	


	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	gladLoadGL();
	

	glEnable(GL_DEPTH_TEST);            // enable depth testing
	glDepthFunc(GL_LESS);

	Shaders shaders("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
	GLuint shaderProgram = shaders.getProgram();
	shaders.use(shaderProgram);

	GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");

	Origin origin;

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Get location of the alpha uniform
	//GLint alphaLoc = glGetUniformLocation(shaderProgram, "uAlpha");

	GLuint  VAO_tri, VBO_tri;
	GLuint  VAO_lines = 0, VBO_lines = 0;

	glGenVertexArrays(1, &VAO_tri);
	glGenBuffers(1, &VBO_tri);
	glGenVertexArrays(1, &VAO_lines);
	glGenBuffers(1, &VBO_lines);

	glBindVertexArray(VAO_tri);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_tri);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

	// use sizeof(Vertex) and offsetof to avoid manual stride/offset math
	GLsizei stride3angles = static_cast<GLsizei>(sizeof(Vertex3angle));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride3angles, (GLvoid*)offsetof(Vertex3angle, pos));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride3angles, (GLvoid*)offsetof(Vertex3angle, color));
	glEnableVertexAttribArray(1);

	// angle attribute (location = 2)
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride3angles, (GLvoid*)offsetof(Vertex3angle, angle));
	glEnableVertexAttribArray(2);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	//glEnable(GL_PROGRAM_POINT_SIZE);
	//glPointSize(95);


	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);      
	glFrontFace(GL_CCW);*/

	while(!glfwWindowShouldClose(window)) {
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		// projection: perspective (change fov/aspect/near/far as needed)
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		float aspect = width > 0 ? (float)width / (float)height : 4.0f / 3.0f;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

		// view: camera at (0,0,3) looking at origin
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		// model: identity or transform your objects (rotate for demo)
		float t = (float)glfwGetTime();
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mvp = projection * view * model;
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	
		

		// draw triangles
		glBindVertexArray(VAO_tri);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(sizeof(triangles) / sizeof(triangles[0])));

		// Draw origin with identity model (static)
		glm::mat4 PV = projection * view;
		glm::mat4 mvpOrigin = PV * glm::mat4(1.0f);
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvpOrigin));
		// ensure same shader is active when origin.draw() relies on this uniform
		glUseProgram(shaderProgram);
		origin.draw();
		
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO_tri);
	glDeleteBuffers(1, &VBO_tri);
	glDeleteProgram(shaderProgram);
	
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}	
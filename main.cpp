#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>

#include <cstddef>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "Shaders.h"
#include "Origin.h"



 struct VertexTriangle {
	GLfloat pos[3];
	GLfloat color[4]; // RGBA
	GLfloat tex[2];   // texture coordinate
};

 using Vertex3angle = VertexTriangle;

 static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
 {
	 // Update GL viewport to new framebuffer size
	 glViewport(0, 0, width, height);

 }

 // Simple WASD camera movement (frame-rate independent).
// Moves cameraPos in world-space using cameraFront and cameraUp.
 static void processKeyboardMove(GLFWwindow* window, glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp, float deltaTime)
 {
	 const float speed = 2.5f; // units per second
	 float velocity = speed * deltaTime;

	 // forward / backward
	 if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraFront * velocity;
	 if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraFront * velocity;

	 // right / left
	 glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
	 if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right * velocity;
	 if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right * velocity;

	 // optional: up / down (space / left ctrl)
	 if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraPos += cameraUp * velocity;
	 if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cameraPos -= cameraUp * velocity;
 }

 static void loadTexturesFromFiles(const char* texturePaths[], int count, GLuint textures[])
 {
	 // flip image vertically to match OpenGL coordinates (common)
	 stbi_set_flip_vertically_on_load(1);

	 for (int i = 0; i < count; ++i) {
		 int w, h, channels;
		 unsigned char* data = stbi_load(texturePaths[i], &w, &h, &channels, 0);
		 if (!data) {
			 std::cerr << "Failed to load texture: " << texturePaths[i] << std::endl;
			 unsigned char fallback[4] = { 255, 0, 255, 255 };
			 glBindTexture(GL_TEXTURE_2D, textures[i]);
			 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, fallback);
			 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			 continue;
		 }

		 GLenum format = GL_RGB;
		 if (channels == 1) format = GL_RED;
		 else if (channels == 3) format = GL_RGB;
		 else if (channels == 4) format = GL_RGBA;

		 glBindTexture(GL_TEXTURE_2D, textures[i]);
		 glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
		 glGenerateMipmap(GL_TEXTURE_2D);

		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		 stbi_image_free(data);
	 }
 }

int main() {
	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Camera state
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	// timing for smooth movement
	float lastFrame = 0.0f;
	float deltaTime = 0.0f;
	

	Vertex3angle triangles [] = {

		// Face 0 - red (opaque)
		{ {  0.0f,  0.6f,  0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.5f, 1.0f } }, // apex
		{ { -0.5f, -0.4f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }, // B0
		{ {  0.5f, -0.4f, -0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } }, // B1

		// Face 1 - green (semi-transparent)
		{ {  0.0f,  0.6f,  0.0f }, { 0.0f, 1.0f, 0.0f, 0.6f }, { 0.5f, 1.0f } }, // apex
		{ {  0.5f, -0.4f, -0.5f }, { 0.0f, 1.0f, 0.0f, 0.6f }, { 0.0f, 0.0f } }, // B1
		{ {  0.0f, -0.4f,  0.6f }, { 0.0f, 1.0f, 0.0f, 0.6f }, { 1.0f, 0.0f } }, // B2

		// Face 2 - blue (opaque)
		{ {  0.0f,  0.6f,  0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.5f, 1.0f } }, // apex
		{ {  0.0f, -0.4f,  0.6f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } }, // B2
		{ { -0.5f, -0.4f, -0.5f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } }  // B0

		
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
	GLint texLoc = glGetUniformLocation(shaderProgram, "uTex");

	Origin origin;

	// Enable blending for transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


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

	// texcoord (location = 3)
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride3angles, (GLvoid*)offsetof(Vertex3angle, tex));
	glEnableVertexAttribArray(3);

	// unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// texture file paths (replace with your images)
	const char* texturePaths[3] = {
		"images/horus.png",
		"images/kamon.png",
		"images/ra.png"
	};
	
	GLuint textures[3];
	glGenTextures(3, textures);

	// load textures using helper function
	loadTexturesFromFiles(texturePaths, 3, textures);

	// set sampler once to texture unit 0 (if shader expects sampler)
	shaders.use(shaderProgram);
	if (texLoc >= 0) glUniform1i(texLoc, 0);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);      
	glFrontFace(GL_CCW);*/

	while(!glfwWindowShouldClose(window)) {


		// timing
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// handle WASD movement (updates cameraPos)
		processKeyboardMove(window, cameraPos, cameraFront, cameraUp, deltaTime);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		// projection: perspective (change fov/aspect/near/far as needed)
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		float aspect = width > 0 ? (float)width / (float)height : 4.0f / 3.0f;
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

		// view: camera at (0,0,3) looking at origin
		/*glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));*/

		// view: derived from camera state
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		// model: identity or transform your objects (rotate for demo)
		float t = (float)glfwGetTime();
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), t * glm::radians(15.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mvp = projection * view * model;
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	
		

		// draw triangles
		glBindVertexArray(VAO_tri);
		// bind and draw per-face textures (one draw call per face)
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glDrawArrays(GL_TRIANGLES, 3, 3);

		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glDrawArrays(GL_TRIANGLES, 6, 3);


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
#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

// Minimal glTF 2.0 loader: parse with fastgltf, upload mesh + baseColor, draw with existing shader layout.
class GltfModel {
public:
	struct Drawable {
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
		GLuint texture = 0;
		GLsizei indexCount = 0;
		GLenum indexType = GL_UNSIGNED_INT;
		glm::mat4 worldMatrix{ 1.0f };
	};

	GltfModel() = default;
	~GltfModel();

	GltfModel(const GltfModel&) = delete;
	GltfModel& operator=(const GltfModel&) = delete;

	bool load(const std::string& gltfPath);
	void draw(GLint mvpLoc, const glm::mat4& viewProj) const;
	void destroy();

	bool isLoaded() const { return loaded; }

private:
	std::vector<Drawable> drawables;
	bool loaded = false;
};

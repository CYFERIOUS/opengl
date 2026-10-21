#include "GltfModel.h"

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace {

struct Vertex {
	GLfloat pos[3];
	GLfloat color[4];
	GLfloat tex[2];
};

glm::mat4 toGlm(const fastgltf::math::fmat4x4& m)
{
	glm::mat4 out(1.0f);
	for (int col = 0; col < 4; ++col) {
		const auto& c = m.col(col);
		out[col][0] = c.x();
		out[col][1] = c.y();
		out[col][2] = c.z();
		out[col][3] = c.w();
	}
	return out;
}

glm::mat4 nodeLocalMatrix(const fastgltf::Node& node)
{
	return std::visit(fastgltf::visitor{
		[](const fastgltf::TRS& trs) {
			return glm::translate(glm::mat4(1.0f), glm::vec3(trs.translation.x(), trs.translation.y(), trs.translation.z()))
				* glm::mat4_cast(glm::quat(trs.rotation.w(), trs.rotation.x(), trs.rotation.y(), trs.rotation.z()))
				* glm::scale(glm::mat4(1.0f), glm::vec3(trs.scale.x(), trs.scale.y(), trs.scale.z()));
		},
		[](const fastgltf::math::fmat4x4& matrix) {
			return toGlm(matrix);
		}
	}, node.transform);
}

fs::path resolveExistingPath(const std::string& path)
{
	fs::path p(path);
	if (fs::exists(p)) {
		return fs::absolute(p);
	}

	const fs::path candidates[] = {
		p,
		fs::path("..") / p,
		fs::path("../..") / p,
		fs::path("../../..") / p,
	};

	for (const auto& c : candidates) {
		if (fs::exists(c)) {
			return fs::absolute(c);
		}
	}
	return {};
}

GLuint createFallbackTexture()
{
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	unsigned char white[4] = { 255, 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

GLuint loadTextureFromFile(const fs::path& path)
{
	stbi_set_flip_vertically_on_load(1);
	int w = 0, h = 0, channels = 0;
	unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &channels, 0);
	if (!data) {
		std::cerr << "Failed to load glTF texture: " << path << std::endl;
		return createFallbackTexture();
	}

	GLenum format = GL_RGB;
	if (channels == 1) format = GL_RED;
	else if (channels == 3) format = GL_RGB;
	else if (channels == 4) format = GL_RGBA;

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);
	return tex;
}

GLuint loadTextureFromImage(const fastgltf::Asset& asset, std::size_t imageIndex, const fs::path& modelDir)
{
	if (imageIndex >= asset.images.size()) {
		return createFallbackTexture();
	}

	const fastgltf::Image& image = asset.images[imageIndex];

	// Prefer URI path (works even if LoadExternalImages was not used).
	if (const auto* uri = std::get_if<fastgltf::sources::URI>(&image.data)) {
		fs::path texPath = modelDir / uri->uri.fspath();
		return loadTextureFromFile(texPath);
	}

	const std::byte* bytes = nullptr;
	std::size_t byteCount = 0;
	if (const auto* arr = std::get_if<fastgltf::sources::Array>(&image.data)) {
		bytes = arr->bytes.data();
		byteCount = arr->bytes.size();
	}
	else if (const auto* vec = std::get_if<fastgltf::sources::Vector>(&image.data)) {
		bytes = vec->bytes.data();
		byteCount = vec->bytes.size();
	}
	else if (const auto* view = std::get_if<fastgltf::sources::ByteView>(&image.data)) {
		bytes = view->bytes.data();
		byteCount = view->bytes.size();
	}

	if (!bytes || byteCount == 0) {
		return createFallbackTexture();
	}

	stbi_set_flip_vertically_on_load(1);
	int w = 0, h = 0, channels = 0;
	unsigned char* data = stbi_load_from_memory(
		reinterpret_cast<const stbi_uc*>(bytes),
		static_cast<int>(byteCount),
		&w, &h, &channels, 0);
	if (!data) {
		std::cerr << "Failed to decode embedded glTF image " << imageIndex << std::endl;
		return createFallbackTexture();
	}

	GLenum format = GL_RGB;
	if (channels == 1) format = GL_RED;
	else if (channels == 3) format = GL_RGB;
	else if (channels == 4) format = GL_RGBA;

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(data);
	return tex;
}

GLuint textureForPrimitive(const fastgltf::Asset& asset, const fastgltf::Primitive& primitive, const fs::path& modelDir)
{
	if (!primitive.materialIndex.has_value()) {
		return createFallbackTexture();
	}

	const auto& material = asset.materials[*primitive.materialIndex];
	if (!material.pbrData.baseColorTexture.has_value()) {
		return createFallbackTexture();
	}

	const std::size_t textureIndex = material.pbrData.baseColorTexture->textureIndex;
	if (textureIndex >= asset.textures.size()) {
		return createFallbackTexture();
	}

	const auto& texture = asset.textures[textureIndex];
	if (!texture.imageIndex.has_value()) {
		return createFallbackTexture();
	}

	return loadTextureFromImage(asset, *texture.imageIndex, modelDir);
}

bool buildDrawable(
	const fastgltf::Asset& asset,
	const fastgltf::Primitive& primitive,
	const glm::mat4& worldMatrix,
	const fs::path& modelDir,
	GltfModel::Drawable& out)
{
	const auto* positionIt = primitive.findAttribute("POSITION");
	if (positionIt == primitive.attributes.end()) {
		std::cerr << "glTF primitive missing POSITION" << std::endl;
		return false;
	}

	const fastgltf::Accessor& positionAccessor = asset.accessors[positionIt->accessorIndex];
	const std::size_t vertexCount = positionAccessor.count;

	std::vector<glm::vec3> positions(vertexCount);
	fastgltf::copyFromAccessor<glm::vec3>(asset, positionAccessor, positions.data());

	std::vector<glm::vec2> texcoords(vertexCount, glm::vec2(0.0f));
	const auto* texIt = primitive.findAttribute("TEXCOORD_0");
	if (texIt != primitive.attributes.end()) {
		fastgltf::copyFromAccessor<glm::vec2>(asset, asset.accessors[texIt->accessorIndex], texcoords.data());
	}

	std::vector<Vertex> vertices(vertexCount);
	for (std::size_t i = 0; i < vertexCount; ++i) {
		vertices[i].pos[0] = positions[i].x;
		vertices[i].pos[1] = positions[i].y;
		vertices[i].pos[2] = positions[i].z;
		vertices[i].color[0] = 1.0f;
		vertices[i].color[1] = 1.0f;
		vertices[i].color[2] = 1.0f;
		vertices[i].color[3] = 1.0f;
		vertices[i].tex[0] = texcoords[i].x;
		vertices[i].tex[1] = texcoords[i].y;
	}

	std::vector<std::uint32_t> indices;
	GLenum indexType = GL_UNSIGNED_INT;
	if (primitive.indicesAccessor.has_value()) {
		const fastgltf::Accessor& indexAccessor = asset.accessors[*primitive.indicesAccessor];
		indices.resize(indexAccessor.count);
		if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedShort) {
			std::vector<std::uint16_t> tmp(indexAccessor.count);
			fastgltf::copyFromAccessor<std::uint16_t>(asset, indexAccessor, tmp.data());
			for (std::size_t i = 0; i < tmp.size(); ++i) {
				indices[i] = tmp[i];
			}
		}
		else if (indexAccessor.componentType == fastgltf::ComponentType::UnsignedByte) {
			std::vector<std::uint8_t> tmp(indexAccessor.count);
			fastgltf::copyFromAccessor<std::uint8_t>(asset, indexAccessor, tmp.data());
			for (std::size_t i = 0; i < tmp.size(); ++i) {
				indices[i] = tmp[i];
			}
		}
		else {
			fastgltf::copyFromAccessor<std::uint32_t>(asset, indexAccessor, indices.data());
		}
	}
	else {
		indices.resize(vertexCount);
		for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(vertexCount); ++i) {
			indices[i] = i;
		}
	}

	out.worldMatrix = worldMatrix;
	out.indexCount = static_cast<GLsizei>(indices.size());
	out.indexType = indexType;
	out.texture = textureForPrimitive(asset, primitive, modelDir);

	glGenVertexArrays(1, &out.vao);
	glGenBuffers(1, &out.vbo);
	glGenBuffers(1, &out.ebo);

	glBindVertexArray(out.vao);
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)), indices.data(), GL_STATIC_DRAW);

	const GLsizei stride = static_cast<GLsizei>(sizeof(Vertex));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, pos)));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, color)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(offsetof(Vertex, tex)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);
	return true;
}

void visitNode(
	const fastgltf::Asset& asset,
	std::size_t nodeIndex,
	const glm::mat4& parentWorld,
	const fs::path& modelDir,
	std::vector<GltfModel::Drawable>& outDrawables)
{
	if (nodeIndex >= asset.nodes.size()) {
		return;
	}

	const fastgltf::Node& node = asset.nodes[nodeIndex];
	const glm::mat4 world = parentWorld * nodeLocalMatrix(node);

	if (node.meshIndex.has_value()) {
		const fastgltf::Mesh& mesh = asset.meshes[*node.meshIndex];
		for (const fastgltf::Primitive& primitive : mesh.primitives) {
			if (primitive.type != fastgltf::PrimitiveType::Triangles) {
				continue;
			}
			GltfModel::Drawable drawable;
			if (buildDrawable(asset, primitive, world, modelDir, drawable)) {
				outDrawables.push_back(drawable);
			}
		}
	}

	for (const std::size_t child : node.children) {
		visitNode(asset, child, world, modelDir, outDrawables);
	}
}

} // namespace

GltfModel::~GltfModel()
{
	destroy();
}

void GltfModel::destroy()
{
	for (Drawable& d : drawables) {
		if (d.vao) glDeleteVertexArrays(1, &d.vao);
		if (d.vbo) glDeleteBuffers(1, &d.vbo);
		if (d.ebo) glDeleteBuffers(1, &d.ebo);
		if (d.texture) glDeleteTextures(1, &d.texture);
		d = {};
	}
	drawables.clear();
	loaded = false;
}

bool GltfModel::load(const std::string& gltfPath)
{
	destroy();

	const fs::path resolved = resolveExistingPath(gltfPath);
	if (resolved.empty()) {
		std::cerr << "glTF file not found (check working directory): " << gltfPath << std::endl;
		return false;
	}

	const fs::path modelDir = resolved.parent_path();
	std::cout << "Loading glTF: " << resolved << std::endl;

	auto data = fastgltf::GltfDataBuffer::FromPath(resolved);
	if (!data) {
		std::cerr << "Failed to read glTF bytes: "
			<< fastgltf::getErrorMessage(data.error()) << std::endl;
		return false;
	}

	fastgltf::Parser parser;
	auto assetExpected = parser.loadGltf(
		data.get(),
		modelDir,
		fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages);

	if (!assetExpected) {
		std::cerr << "Failed to parse glTF: "
			<< fastgltf::getErrorMessage(assetExpected.error()) << std::endl;
		return false;
	}

	fastgltf::Asset& asset = assetExpected.get();

	if (!asset.scenes.empty()) {
		const std::size_t sceneIndex = asset.defaultScene.has_value() ? *asset.defaultScene : 0;
		const fastgltf::Scene& scene = asset.scenes[sceneIndex];
		for (const std::size_t rootNode : scene.nodeIndices) {
			visitNode(asset, rootNode, glm::mat4(1.0f), modelDir, drawables);
		}
	}
	else {
		// No scenes: upload every mesh with identity transform.
		for (const fastgltf::Mesh& mesh : asset.meshes) {
			for (const fastgltf::Primitive& primitive : mesh.primitives) {
				if (primitive.type != fastgltf::PrimitiveType::Triangles) {
					continue;
				}
				Drawable drawable;
				if (buildDrawable(asset, primitive, glm::mat4(1.0f), modelDir, drawable)) {
					drawables.push_back(drawable);
				}
			}
		}
	}

	loaded = !drawables.empty();
	if (!loaded) {
		std::cerr << "glTF loaded but no triangle primitives were uploaded" << std::endl;
	}
	else {
		std::cout << "Uploaded " << drawables.size() << " glTF drawable(s)" << std::endl;
	}
	return loaded;
}

void GltfModel::draw(GLint mvpLoc, const glm::mat4& viewProj) const
{
	if (!loaded) {
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	for (const Drawable& d : drawables) {
		const glm::mat4 mvp = viewProj * d.worldMatrix;
		if (mvpLoc >= 0) {
			glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		}
		glBindTexture(GL_TEXTURE_2D, d.texture);
		glBindVertexArray(d.vao);
		glDrawElements(GL_TRIANGLES, d.indexCount, d.indexType, nullptr);
	}
	glBindVertexArray(0);
}

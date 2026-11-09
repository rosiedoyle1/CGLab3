#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>

#include <cmath>
#include <iostream>
#include <vector>

namespace {

const int windowWidth = 1024;
const int windowHeight = 768;
const float pi = 3.14159265359f;
const glm::vec3 lightPosition(-105.0f, 128.0f, -25.0f);
const glm::vec3 lightIntensity(24000.0f, 19500.0f, 14000.0f);
const glm::vec3 cameraTarget(0.0f, -50.0f, 0.0f);
glm::vec3 cameraPosition(650.0f, 500.0f, 850.0f);
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraDistance = 0.0f;
bool rotatingCamera = false;
double previousMouseX = 0.0;
double previousMouseY = 0.0;

void updateCameraPosition() {
	cameraPosition = cameraTarget + cameraDistance * glm::vec3(
		std::cos(cameraPitch) * std::cos(cameraYaw),
		std::sin(cameraPitch),
		std::cos(cameraPitch) * std::sin(cameraYaw));
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	rotatingCamera = action == GLFW_PRESS;
	if (rotatingCamera) {
		glfwGetCursorPos(window, &previousMouseX, &previousMouseY);
	}
}

void cursorPositionCallback(GLFWwindow*, double x, double y) {
	if (!rotatingCamera) return;
	const float sensitivity = 0.005f;
	cameraYaw += static_cast<float>(x - previousMouseX) * sensitivity;
	cameraPitch -= static_cast<float>(y - previousMouseY) * sensitivity;
	if (cameraPitch > 1.2f) cameraPitch = 1.2f;
	if (cameraPitch < -1.2f) cameraPitch = -1.2f;
	previousMouseX = x;
	previousMouseY = y;
	updateCameraPosition();
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		cameraPosition = glm::vec3(650.0f, 500.0f, 850.0f);
		glm::vec3 offset = cameraPosition - cameraTarget;
		cameraDistance = glm::length(offset);
		cameraYaw = std::atan2(offset.z, offset.x);
		cameraPitch = std::asin(offset.y / cameraDistance);
	}
}

struct SceneMesh {
	std::vector<GLfloat> positions;
	std::vector<GLfloat> colors;
	std::vector<GLfloat> normals;
	std::vector<GLfloat> textureCoordinates;

	void addTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color,
		bool useWoodTexture = false) {
		glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
		const glm::vec3 vertices[] = { a, b, c };
		for (int i = 0; i < 3; ++i) {
			positions.push_back(vertices[i].x);
			positions.push_back(vertices[i].y);
			positions.push_back(vertices[i].z);
			colors.push_back(color.r);
			colors.push_back(color.g);
			colors.push_back(color.b);
			normals.push_back(normal.x);
			normals.push_back(normal.y);
			normals.push_back(normal.z);

			if (std::fabs(normal.y) > 0.5f) {
				textureCoordinates.push_back(vertices[i].x / 200.0f);
				textureCoordinates.push_back(vertices[i].z / 200.0f);
			}
			else if (std::fabs(normal.x) > 0.5f) {
				textureCoordinates.push_back(vertices[i].z / 200.0f);
				textureCoordinates.push_back(vertices[i].y / 200.0f);
			}
			else {
				textureCoordinates.push_back(vertices[i].x / 200.0f);
				textureCoordinates.push_back(vertices[i].y / 200.0f);
			}
			textureCoordinates.push_back(useWoodTexture ? 1.0f : 0.0f);
		}
	}

	void addQuad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
		glm::vec3 color, bool useWoodTexture = false) {
		addTriangle(a, b, c, color, useWoodTexture);
		addTriangle(a, c, d, color, useWoodTexture);
	}

	void addCuboid(glm::vec3 minimum, glm::vec3 maximum, glm::vec3 color,
		bool useWoodTexture = false) {
		const float x0 = minimum.x, y0 = minimum.y, z0 = minimum.z;
		const float x1 = maximum.x, y1 = maximum.y, z1 = maximum.z;
		addQuad({x1, y0, z0}, {x1, y1, z0}, {x1, y1, z1}, {x1, y0, z1}, color, useWoodTexture);
		addQuad({x0, y0, z1}, {x0, y1, z1}, {x0, y1, z0}, {x0, y0, z0}, color, useWoodTexture);
		addQuad({x0, y1, z1}, {x1, y1, z1}, {x1, y1, z0}, {x0, y1, z0}, color, useWoodTexture);
		addQuad({x0, y0, z0}, {x1, y0, z0}, {x1, y0, z1}, {x0, y0, z1}, color, useWoodTexture);
		addQuad({x0, y0, z1}, {x1, y0, z1}, {x1, y1, z1}, {x0, y1, z1}, color, useWoodTexture);
		addQuad({x1, y0, z0}, {x0, y0, z0}, {x0, y1, z0}, {x1, y1, z0}, color, useWoodTexture);
	}

	void addCylinder(glm::vec3 start, glm::vec3 end, float radius, glm::vec3 color, int segments = 20) {
		glm::vec3 axis = glm::normalize(end - start);
		glm::vec3 reference = std::fabs(axis.y) < 0.9f
			? glm::vec3(0.0f, 1.0f, 0.0f)
			: glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 u = glm::normalize(glm::cross(axis, reference));
		glm::vec3 v = glm::normalize(glm::cross(axis, u));

		for (int i = 0; i < segments; ++i) {
			float angle0 = 2.0f * pi * i / segments;
			float angle1 = 2.0f * pi * (i + 1) / segments;
			glm::vec3 radial0 = radius * (u * std::cos(angle0) + v * std::sin(angle0));
			glm::vec3 radial1 = radius * (u * std::cos(angle1) + v * std::sin(angle1));
			addQuad(start + radial0, end + radial0, end + radial1, start + radial1, color);
			addTriangle(start, start + radial1, start + radial0, color);
			addTriangle(end, end + radial0, end + radial1, color);
		}
	}

	void addLampShade(glm::vec3 top, glm::vec3 bottom, float topRadius,
		float bottomRadius, glm::vec3 color, int segments = 24) {
		glm::vec3 axis = glm::normalize(bottom - top);
		glm::vec3 reference(0.0f, 0.0f, 1.0f);
		glm::vec3 u = glm::normalize(glm::cross(axis, reference));
		glm::vec3 v = glm::normalize(glm::cross(axis, u));

		for (int i = 0; i < segments; ++i) {
			float angle0 = 2.0f * pi * i / segments;
			float angle1 = 2.0f * pi * (i + 1) / segments;
			glm::vec3 direction0 = u * std::cos(angle0) + v * std::sin(angle0);
			glm::vec3 direction1 = u * std::cos(angle1) + v * std::sin(angle1);
			addQuad(top + topRadius * direction0, bottom + bottomRadius * direction0,
				bottom + bottomRadius * direction1, top + topRadius * direction1, color);
		}
	}

	void addBookPage(float xOuter, float xHinge, float outerY, float hingeY,
		float zFront, float zBack, glm::vec3 color) {
		addQuad({xOuter, outerY, zFront}, {xOuter, outerY, zBack},
			{xHinge, hingeY, zBack}, {xHinge, hingeY, zFront}, color);

		for (int line = 0; line < 6; ++line) {
			float z = zFront + 18.0f + line * 16.0f;
			float yOuter = outerY + 1.0f;
			float yHinge = hingeY + 1.0f;
			glm::vec3 ink(0.48f, 0.40f, 0.29f);
			if (xOuter < xHinge) {
				addQuad({xOuter + 12.0f, yOuter, z}, {xOuter + 12.0f, yOuter, z + 1.2f},
					{xHinge - 12.0f, yHinge, z + 1.2f}, {xHinge - 12.0f, yHinge, z}, ink);
			}
			else {
				addQuad({xHinge + 12.0f, yHinge, z}, {xHinge + 12.0f, yHinge, z + 1.2f},
					{xOuter - 12.0f, yOuter, z + 1.2f}, {xOuter - 12.0f, yOuter, z}, ink);
			}
		}
	}

	void build() {
		const glm::vec3 tabletop(0.34f, 0.19f, 0.10f);
		const glm::vec3 legColor(0.22f, 0.12f, 0.07f);
		const glm::vec3 metal(0.12f, 0.13f, 0.14f);

		addCuboid({-360.0f, 0.0f, -220.0f}, {360.0f, 24.0f, 220.0f}, tabletop, true);
		addCuboid({-330.0f, -370.0f, -195.0f}, {-290.0f, 0.0f, -155.0f}, legColor, true);
		addCuboid({290.0f, -370.0f, -195.0f}, {330.0f, 0.0f, -155.0f}, legColor, true);
		addCuboid({-330.0f, -370.0f, 155.0f}, {-290.0f, 0.0f, 195.0f}, legColor, true);
		addCuboid({290.0f, -370.0f, 155.0f}, {330.0f, 0.0f, 195.0f}, legColor, true);

		addCuboid({-1000.0f, -371.0f, -520.0f}, {1000.0f, -370.0f, 1000.0f},
			glm::vec3(0.24f, 0.22f, 0.19f));
		addQuad({-1000.0f, -370.0f, -520.0f}, {1000.0f, -370.0f, -520.0f},
			{1000.0f, 800.0f, -520.0f}, {-1000.0f, 800.0f, -520.0f},
			glm::vec3(0.28f, 0.27f, 0.24f));

		addCylinder({-220.0f, 24.0f, -25.0f}, {-220.0f, 39.0f, -25.0f},
			58.0f, metal, 32);
		addCylinder({-220.0f, 39.0f, -25.0f}, {-164.0f, 155.0f, -25.0f},
			8.0f, metal);
		addCylinder({-164.0f, 155.0f, -25.0f}, {-108.0f, 184.0f, -25.0f},
			7.0f, metal);
		addLampShade({-108.0f, 184.0f, -25.0f}, {-105.0f, 135.0f, -25.0f},
			24.0f, 70.0f, glm::vec3(0.11f, 0.10f, 0.085f));
		addCylinder({-105.0f, 133.0f, -25.0f}, {-105.0f, 137.0f, -25.0f},
			38.0f, glm::vec3(0.95f, 0.77f, 0.42f), 24);

		addCuboid({-55.0f, 25.0f, 15.0f}, {215.0f, 31.0f, 185.0f},
			glm::vec3(0.20f, 0.08f, 0.045f));
		addBookPage(-48.0f, 80.0f, 33.0f, 47.0f, 23.0f, 177.0f,
			glm::vec3(0.82f, 0.74f, 0.56f));
		addBookPage(208.0f, 80.0f, 33.0f, 47.0f, 23.0f, 177.0f,
			glm::vec3(0.88f, 0.81f, 0.65f));
	}
};

struct SceneRenderer {
	GLuint vertexArray = 0;
	GLuint positionBuffer = 0;
	GLuint colorBuffer = 0;
	GLuint normalBuffer = 0;
	GLuint textureCoordinateBuffer = 0;
	GLuint woodTexture = 0;
	GLuint litProgram = 0;
	GLuint depthProgram = 0;
	GLuint cameraMatrixID = 0;
	GLuint lightSpaceMatrixID = 0;
	GLuint lightPositionID = 0;
	GLuint lightIntensityID = 0;
	GLuint ambientIntensityID = 0;
	GLuint shadowMapID = 0;
	GLuint woodTextureID = 0;
	GLuint depthMatrixID = 0;
	GLuint shadowFramebuffer = 0;
	GLuint shadowTexture = 0;
	int shadowWidth = 0;
	int shadowHeight = 0;

	bool initialize(const SceneMesh& mesh) {
		litProgram = LoadShadersFromFile("../lab3/myscene.vert", "../lab3/myscene.frag");
		depthProgram = LoadShadersFromFile("../lab3/depth.vert", "../lab3/depth.frag");
		if (!litProgram || !depthProgram) {
			std::cerr << "Failed to load scene shaders." << std::endl;
			return false;
		}

		cameraMatrixID = glGetUniformLocation(litProgram, "MVP");
		lightSpaceMatrixID = glGetUniformLocation(litProgram, "lightSpaceMatrix");
		lightPositionID = glGetUniformLocation(litProgram, "lightPosition");
		lightIntensityID = glGetUniformLocation(litProgram, "lightIntensity");
		ambientIntensityID = glGetUniformLocation(litProgram, "ambientIntensity");
		shadowMapID = glGetUniformLocation(litProgram, "shadowMap");
		woodTextureID = glGetUniformLocation(litProgram, "woodTexture");
		depthMatrixID = glGetUniformLocation(depthProgram, "MVP");

		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		glGenBuffers(1, &positionBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(GLfloat),
			mesh.positions.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &colorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.colors.size() * sizeof(GLfloat),
			mesh.colors.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &normalBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(GLfloat),
			mesh.normals.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &textureCoordinateBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, textureCoordinateBuffer);
		glBufferData(GL_ARRAY_BUFFER, mesh.textureCoordinates.size() * sizeof(GLfloat),
			mesh.textureCoordinates.data(), GL_STATIC_DRAW);

		std::vector<unsigned char> woodPixels(256 * 256 * 3);
		for (int y = 0; y < 256; ++y) {
			for (int x = 0; x < 256; ++x) {
				float grain = 0.875f + 0.12f * std::sin(
					2.0f * pi * y / 16.0f + 0.7f * std::sin(2.0f * pi * x / 96.0f));
				unsigned char value = static_cast<unsigned char>(255.0f * grain);
				std::size_t offset = static_cast<std::size_t>((y * 256 + x) * 3);
				woodPixels[offset] = value;
				woodPixels[offset + 1] = value;
				woodPixels[offset + 2] = value;
			}
		}
		glGenTextures(1, &woodTexture);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB,
			GL_UNSIGNED_BYTE, woodPixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &shadowFramebuffer);
		glGenTextures(1, &shadowTexture);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowWidth, shadowHeight,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		const GLfloat border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			shadowTexture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cerr << "Scene shadow framebuffer is incomplete." << std::endl;
			return false;
		}

		return true;
	}

	void bindAttribute(GLuint index, GLuint buffer) {
		glEnableVertexAttribArray(index);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	GLsizei vertexCount(const SceneMesh& mesh) const {
		return static_cast<GLsizei>(mesh.positions.size() / 3);
	}

	void renderDepth(const SceneMesh& mesh, const glm::mat4& lightMatrix) {
		glUseProgram(depthProgram);
		glBindVertexArray(vertexArray);
		bindAttribute(0, positionBuffer);
		glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &lightMatrix[0][0]);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount(mesh));
		glDisableVertexAttribArray(0);
	}

	void render(const SceneMesh& mesh, const glm::mat4& cameraMatrix,
		const glm::mat4& lightMatrix) {
		glUseProgram(litProgram);
		glBindVertexArray(vertexArray);
		bindAttribute(0, positionBuffer);
		bindAttribute(1, colorBuffer);
		bindAttribute(2, normalBuffer);
		bindAttribute(3, textureCoordinateBuffer);
		glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
		glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightMatrix[0][0]);
		glUniform3fv(lightPositionID, 1, &lightPosition[0]);
		glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
		glUniform3f(ambientIntensityID, 0.035f, 0.032f, 0.028f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowTexture);
		glUniform1i(shadowMapID, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glUniform1i(woodTextureID, 1);
		glDrawArrays(GL_TRIANGLES, 0, vertexCount(mesh));
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
	}

	void cleanup() {
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &colorBuffer);
		glDeleteBuffers(1, &normalBuffer);
		glDeleteBuffers(1, &textureCoordinateBuffer);
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteProgram(litProgram);
		glDeleteProgram(depthProgram);
		glDeleteFramebuffers(1, &shadowFramebuffer);
		glDeleteTextures(1, &shadowTexture);
		glDeleteTextures(1, &woodTexture);
	}
};

} // namespace

int main() {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Study Desk", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGL(glfwGetProcAddress)) {
		std::cerr << "Failed to initialize OpenGL." << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	SceneMesh scene;
	scene.build();
	SceneRenderer renderer;
	glfwGetFramebufferSize(window, &renderer.shadowWidth, &renderer.shadowHeight);
	if (!renderer.initialize(scene)) {
		renderer.cleanup();
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.12f, 0.13f, 0.16f, 1.0f);

	glm::vec3 initialOffset = cameraPosition - cameraTarget;
	cameraDistance = glm::length(initialOffset);
	cameraYaw = std::atan2(initialOffset.z, initialOffset.x);
	cameraPitch = std::asin(initialOffset.y / cameraDistance);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetKeyCallback(window, keyCallback);
	const glm::mat4 projection = glm::perspective(glm::radians(45.0f),
		static_cast<float>(windowWidth) / windowHeight, 1.0f, 3000.0f);
	const glm::vec3 lightTarget(-30.0f, 0.0f, 30.0f);
	const glm::mat4 lightView = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 0.0f, 1.0f));
	const glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f),
		static_cast<float>(renderer.shadowWidth) / renderer.shadowHeight, 10.0f, 1500.0f);
	const glm::mat4 lightMatrix = lightProjection * lightView;

	while (!glfwWindowShouldClose(window)) {
		glm::mat4 view = glm::lookAt(cameraPosition, cameraTarget,
			glm::vec3(0.0f, 1.0f, 0.0f));
		glBindFramebuffer(GL_FRAMEBUFFER, renderer.shadowFramebuffer);
		glViewport(0, 0, renderer.shadowWidth, renderer.shadowHeight);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0f, 4.0f);
		renderer.renderDepth(scene, lightMatrix);
		glDisable(GL_POLYGON_OFFSET_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, renderer.shadowWidth, renderer.shadowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderer.render(scene, projection * view, lightMatrix);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	renderer.cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//	Forward declaration:
int init(GLFWwindow*& window);			//	Try to find out what the difference is between (Class* Param), and (Class* &Param).
void processInput(GLFWwindow* window);
void createGeometry(GLuint& VAO, GLuint& VBO, int& size, int& indices);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
GLuint loadTexture(const char* path, int comp = 0);
void renderSkybox();
void renderTerrain();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID);


//	Window callbacks:
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool keys[1024];

//	Util:
void loadFile(const char* filename, char*& output);

//	Program ID's
GLuint simpleProgram, skyProgram, terrainProgram, modelProgram;

//	Properties
const int width = 1280, height = 720;

//	World data
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(0, 100, 0);

//	Define vertex buffers, (I think.)
GLuint boxVAO, boxEBO;
int boxSize, boxIndexCount;

//	Matrices!
glm::mat4 view, projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//	Terrain data:
GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, rock, snow;

Model* backpack;

int main()
{
	//	Initialize the window.
	GLFWwindow* window = NULL;
	if (init(window) < 0) return -1;

	//	Checking OpenGL and GLSL versions, and active GPU
	std::cout << "OpenGL version: "	<< glGetString(GL_VERSION)					<< std::endl;
	std::cout << "GLSL version: "	<< glGetString(GL_SHADING_LANGUAGE_VERSION)	<< std::endl;
	std::cout << "Renderer: "		<< glGetString(GL_RENDERER)					<< std::endl;

	//	Setting framerate cap.
	glfwSwapInterval(1);

	//	Flip texture UV's
	stbi_set_flip_vertically_on_load(true);

	//	Load resources.
	createShaders();
	createGeometry(boxVAO, boxEBO, boxSize, boxIndexCount);

	terrainVAO		= GeneratePlane("textures/heightmap.png", heightmapTexture, GL_RGBA, 4, 250.0f, 8.0f, terrainIndexCount, heightmapID);
	heightNormalID	= loadTexture("textures/heightnormal.png");

	dirt	= loadTexture("textures/dirt.jpg");
	sand	= loadTexture("textures/sand.jpg");
	grass	= loadTexture("textures/grass.png", 4);
	rock	= loadTexture("textures/rock.jpg");
	snow	= loadTexture("textures/snow.jpg");

	backpack = new Model("models/backpack/backpack.obj");

	//	Create a viewport.
	glViewport(0, 0, width, height);

	//	Assign matrices.
	view		= glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projection	= glm::perspective(glm::radians(75.0f), width / (float)height, 0.1f, 10000.0f);

	//	Game loop.
	while (!glfwWindowShouldClose(window))
	{
		//	Input.
		processInput(window);

		//	Rendering.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderSkybox();
		renderTerrain();

		float t = glfwGetTime();
		renderModel(backpack, glm::vec3(100, 100, 100), glm::vec3(0, t, 0), glm::vec3(10, 10, 10));

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

void renderSkybox()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);

	glUseProgram(skyProgram);

	glm::mat4 world	= glm::mat4(1.0f);
	world			= glm::translate(world, cameraPosition);
	world			= glm::scale(world, glm::vec3(100, 100, 100));

	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glBindVertexArray(boxVAO);
	glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH);
}

void renderTerrain()
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(terrainProgram);

	glm::mat4 world	= glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


	//float t = glfwGetTime();
	//lightDirection = glm::normalize(glm::vec3(glm::sin(t), -0.5f, glm::cos(t)));
	glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightNormalID);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirt);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sand);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grass);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, rock);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snow);

	//	Rendering
	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
}

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID)
{
	int width, height, channels;
	data = nullptr;
	if (heightmap != nullptr)
	{
		data = stbi_load(heightmap, &width, &height, &channels, comp);
		if (data)
		{
			glGenTextures(1, &heightmapID);
			glBindTexture(GL_TEXTURE_2D, heightmapID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	int stride				= 8;
	float* vertices			= new float[(width * height) * stride];
	unsigned int* indices	= new unsigned int[(width - 1) * (height - 1) * 6];

	int index = 0;
	for (int i = 0; i < (width * height); i++) 
	{
		// Calculate x/z values
		int x = i % width;
		int z = i / width;

		float texHeight = (float)data[i * comp];

		// Set position
		vertices[index++] = x * xzScale;
		vertices[index++] = (texHeight / 255.0f) * hScale;
		vertices[index++] = z * xzScale;

		// Set normal
		vertices[index++] = 0;
		vertices[index++] = 1;
		vertices[index++] = 0;

		// Set uv
		vertices[index++] = x / (float)width;
		vertices[index++] = z / (float)height;
	}

	// OPTIONAL TODO: Calculate normal
	// TODO: Set normal

	index = 0;
	for (int i = 0; i < (width - 1) * (height - 1); i++)
	{
		// Calculate x/z values
		int x = i % (width - 1);
		int z = i / (width - 1);

		int vertex = z * width + x;

		indices[index++] = vertex;
		indices[index++] = vertex + width;
		indices[index++] = vertex + width + 1;

		indices[index++] = vertex;
		indices[index++] = vertex + width + 1;
		indices[index++] = vertex + 1;
	}

	unsigned int vertSize = (width * height) * stride * sizeof(float);
	indexCount = ((width - 1) * (height - 1) * 6);

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex information!
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	delete[] vertices;
	delete[] indices;

	// stbi_image_free(data);

	return VAO;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	bool camChanged = false;

	if (keys[GLFW_KEY_W])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, 10);
		camChanged = true;
	}
	if (keys[GLFW_KEY_S])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, -10);
		camChanged = true;
	}
	if (keys[GLFW_KEY_A])
	{
		cameraPosition += camQuat * glm::vec3(10, 0, 0);
		camChanged = true;
	}
	if (keys[GLFW_KEY_D])
	{
		cameraPosition += camQuat * glm::vec3(-10, 0, 0);
		camChanged = true;
	}

	if (camChanged)
	{
		glm::vec3 camForward	= camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp			= camQuat * glm::vec3(0, 1, 0);

		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
	}
}

int init(GLFWwindow*& window)
{
	//	Initialize GLFW.
	if (!glfwInit())
	{
		std::cout << "ERROR: GLFW initialization failed." << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//	Create a GLFW window.
	window = glfwCreateWindow(width, height, "Graphics Program", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "ERROR: Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}

	//	Register callbacks.
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);

	//	Load GLAD.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "ERROR: Failed to load Glad." << std::endl;
		return -1;
	}

	return 0;
}

void createGeometry(GLuint& VAO, GLuint& EBO, int& size, int& numIndices)
{
	// need 24 vertices for normal/uv-mapped Cube
	float vertices[] = {
		// positions            //colors            // tex coords   // normals          //tangents      //bitangents
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
	};

	unsigned int indices[] = // note that we start from 0!
	{
		// DOWN
		0, 1, 2,   // first triangle
		0, 2, 3,    // second triangle
		// BACK
		14, 6, 7,   // first triangle
		14, 7, 15,    // second triangle
		// RIGHT
		20, 4, 5,   // first triangle
		20, 5, 21,    // second triangle
		// LEFT
		16, 8, 9,   // first triangle
		16, 9, 17,    // second triangle
		// FRONT
		18, 10, 11,   // first triangle
		18, 11, 19,    // second triangle
		// UP
		22, 12, 13,   // first triangle
		22, 13, 23,    // second triangle
	};

	//	Calculating the size and indices.
	int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

	size = sizeof(vertices) / stride;
	numIndices = sizeof(indices) / sizeof(int);

	//	Creating the VAO index, and binding it to create it's configuration.
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//	Create buffer, bind it & assign vertices ot it.
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//	Set layout of vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);

}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	float x = (float)xpos;
	float y = (float)ypos;

	if (firstMouse)
	{
		lastX		= x;
		lastY		= y;
		firstMouse	= false;
	}

	float dx = x - lastX;
	float dy = y - lastY;
	lastX = x;
	lastY = y;

	camYaw		-= dx;
	camPitch	= glm::clamp(camPitch + dy, -90.0f, 90.0f);
	if (camYaw > 180.0f)	camYaw -= 360.0f;
	if (camYaw < -180.0f)	camYaw += 360.0f;

	camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

	glm::vec3 camForward	= camQuat * glm::vec3(0, 0, 1);
	glm::vec3 camUp			= camQuat * glm::vec3(0, 1, 0);

	view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void createShaders()
{
	//	Creating program for simple cube.
	createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "diffuseTex"),	0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"),	1);

	//	Creating program for skybox
	createProgram(skyProgram, "shaders/skyVertex.shader", "shaders/skyFragment.shader");

	//	Creating program for terrain.
	createProgram(terrainProgram, "shaders/terrainVertex.shader", "shaders/terrainFragment.shader");
	glUseProgram(terrainProgram);
	glUniform1i(glGetUniformLocation(terrainProgram, "diffuseTex"),	0);
	glUniform1i(glGetUniformLocation(terrainProgram, "normalTex"),	1);
	glUniform1i(glGetUniformLocation(terrainProgram, "dirt"),		2);
	glUniform1i(glGetUniformLocation(terrainProgram, "sand"),		3);
	glUniform1i(glGetUniformLocation(terrainProgram, "grass"),		4);
	glUniform1i(glGetUniformLocation(terrainProgram, "rock"),		5);
	glUniform1i(glGetUniformLocation(terrainProgram, "snow"),		6);

	//	Creating program for loaded model.
	createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");
	glUseProgram(modelProgram);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"),		0);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"),	1);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_norma1l"),		2);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"),	3);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"),			4);
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment) 
{
	char* vertexSrc;
	char* fragmentSrc;

	int succes;
	char infolog[512];

	loadFile(vertex, vertexSrc);
	loadFile(fragment, fragmentSrc);

	GLuint vertexShaderID, fragmentShaderID;

	//	Creating vertex shader.
	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShaderID);

	//	Vertex shader error handling.
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &succes);
	if (!succes)
	{
		glGetShaderInfoLog(vertexShaderID, 512, nullptr, infolog);
		std::cout << "ERROR COMPILING VERTEX SHADER\n" << infolog << std::endl;
	}

	//	Creating fragment shader.
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShaderID);

	//	Fragment shader error handling.
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &succes);
	if (!succes)
	{
		glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infolog);
		std::cout << "ERROR COMPILING FRAGMENT SHADER\n" << infolog << std::endl;;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &succes);
	if (!succes)
	{
		glGetProgramInfoLog(programID, 512, nullptr, infolog);
		std::cout << "ERROR LINKING PROGRAM\n" << infolog << std::endl;;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	delete vertexSrc;
	delete fragmentSrc;
}

void loadFile(const char* filename, char*& output) 
{
	//	Open the file.
	std::ifstream file(filename, std::ios::binary);

	if (file.is_open()) 
	{
		//	Get length of file.
		file.seekg(0, file.end);
		int length = file.tellg();
		file.seekg(0, file.beg);

		//	Allocate memory for the char pointer.
		output = new char[length + 1];

		//	Read data as block.
		file.read(output, length);

		//	Add null terminator to end of pointer.
		output[length] = '\0';

		//	Close the file.	
		file.close();
	}
	else
	{
		//	If the file failed to opne, set the char pointer to NULL.
		output = NULL;
	}
}

GLuint loadTexture(const char* path, int comp)
{
	//	Generate and bind a texture. (Whatever that means ;_:)
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//	Setting texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//	Loading the texture.
	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);

	//	Setting data.
	if (data)
	{
		if (comp != 0) numChannels = comp;

		if		(numChannels == 3)	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (numChannels == 4)	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Error loading texture: " << path << "." << std::endl;
;	}

	//	Unloading texture.
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	//	Return it.
	return textureID;
}

void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) 
{
	//	Configuring GPU options I think??
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(modelProgram);

	//	Passing translation data into the program.
	glm::mat4 world = glm::mat4(1.0f);

	world = glm::translate(world, pos);
	world = world * glm::toMat4(glm::quat(rot));
	world = glm::scale(world, scale);

	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"),			1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"),			1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"),	1, GL_FALSE, glm::value_ptr(projection));

	//	Passing world light information into the render program.
	glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	//	Calling the model's render program.
	model->Draw(modelProgram);
}
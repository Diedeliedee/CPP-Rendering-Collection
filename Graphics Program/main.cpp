#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"
#include "model.h"
#include "portal.h"
#include "terrain.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace util;

#pragma region Forward Declaration

//	Main:
int init(GLFWwindow*& window);			//	Try to find out what the difference is between (Class* Param), and (Class* &Param).

//	Generating:
void createGeometry(GLuint& VAO, GLuint& VBO, int& size, int& indices);

//	Rendering:
void renderSkybox();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

//	Input:
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//	Util:
void createShaders();

#pragma endregion

#pragma region Member Variables

//	Window:
const int width = 1280, height = 720;

//	Program ID's:
GLuint simpleProgram, skyProgram, modelProgram;

//	Input:
bool keys[1024];

//	World Data:
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(0, 100, 0);

//	Define vertex buffers, (I think.)
GLuint boxVAO, boxEBO;
int boxSize, boxIndexCount;

//	Camera variables:
glm::mat4 view, projection;
float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//	Model Data:
//Model* backpack;

//	Portal:
Terrain* terrain;
Portal* portal;

#pragma endregion

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
	//stbi_set_flip_vertically_on_load(true);

	//	Load resources.
	createShaders();

	//	Creating stuff for the skybox.
	createGeometry(boxVAO, boxEBO, boxSize, boxIndexCount);

	//	Creating stuff for the terrain!


	//	Creating stuff for models.
	//backpack	= new Model("models/backpack/backpack.obj");

	//	Creating a portal.
	terrain		= new Terrain();
	portal		= new Portal(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 100);

	//	Create a viewport.
	glViewport(0, 0, width, height);

	//	Assign matrices.
	view		= glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projection	= glm::perspective(glm::radians(75.0f), width / (float)height, 0.1f, 5000.0f);

	//	Game loop.
	while (!glfwWindowShouldClose(window))
	{
		//	Input.
		processInput(window);

		//	Clearing previous draw.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//	Rendering environment
		renderSkybox();

		//	Rendering models.
		//float t = glfwGetTime();
		//renderModel(backpack, glm::vec3(1000, 100, 1000), glm::vec3(0, 0, 0), glm::vec3(100, 100, 100));
		terrain->draw(view, projection, lightDirection, cameraPosition);
		portal->draw();

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

#pragma region Setup

/// <summary>
/// Initializes GLFW window.
/// </summary>
/// <param name="window">Reference to the created window</param>
/// <returns>Callback integer stating the result of initialization.</returns>
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

/// <summary>
/// Function to initialize al the programs! Creates shader instances, and configures their variables to certain destinations.
/// </summary>
void createShaders()
{
	//	Creating program for simple cube.
	createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "diffuseTex"),	0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"),	1);

	//	Creating program for skybox
	createProgram(skyProgram, "shaders/skyVertex.shader", "shaders/skyFragment.shader");

	//	Creating program for loaded model.
	createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");
	glUseProgram(modelProgram);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"),		0);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"),	1);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_norma1l"),		2);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"),	3);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"),			4);
}

#pragma endregion

#pragma region Generating Geometry


/// <summary>
/// Function that creates a box.
/// </summary>
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

#pragma endregion

#pragma region Rendering

void renderSkybox()
{
	//	Configuring options.
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);

	//	Setting current program.
	glUseProgram(skyProgram);

	//	Creating world matrix.
	glm::mat4 world	= glm::mat4(1.0f);
	world			= glm::translate(world, cameraPosition);
	world			= glm::scale(world, glm::vec3(100, 100, 100));

	//	Injecting projection data.
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//	Injecting relevant vectors.
	glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	//	Drawing!
	glBindVertexArray(boxVAO);
	glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

	//	Re-enabling some stuff.
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH);
}



void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale) 
{
	//	Enabling blending.
	//glEnable(GL_BLEND);

	//	Alpha blend.
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	Additive blend.
	//glBlendFunc(GL_ONE, GL_ONE);
	//	Soft additive blend.
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
	//  Multiply blend.
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);
	//  Double multiply blend.
	//glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

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

	//	Disabling blending.
	glDisable(GL_BLEND);
}

#pragma endregion

#pragma region Input

void processInput(GLFWwindow* window)
{
	//	Close window if escape is pressed.
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	//	Define false cam changed.
	bool camChanged = false;

	//	Move the camera if a key is pressed!
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

	//	If the camera has moved, recalculate the view matrix.
	if (camChanged)
	{
		glm::vec3 camForward	= camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp			= camQuat * glm::vec3(0, 1, 0);

		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
	}
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

#pragma endregion

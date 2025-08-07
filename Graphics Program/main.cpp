#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"

#include "skybox.h"
#include "terrain.h"
#include "model.h"
#include "portal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace util;

#pragma region Forward Declaration

//	Main:
int init(GLFWwindow*& window);			//	Try to find out what the difference is between (Class* Param), and (Class* &Param).

//	Rendering:
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
GLuint simpleProgram, modelProgram;

//	Input:
bool keys[1024];

//	World Data:
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(0, 100, 0);

//	Camera variables:
glm::mat4 view, projection;
float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//	Model Data:
//Model* backpack;

//	Portal:
Skybox*		skybox;
Terrain*	terrain;
Portal*		portal;

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

	//	Creating stuff for models.
	//backpack	= new Model("models/backpack/backpack.obj");

	//	Creating a portal.
	skybox		= new Skybox();
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

		//	Rendering models.
		//float t = glfwGetTime();
		//renderModel(backpack, glm::vec3(1000, 100, 1000), glm::vec3(0, 0, 0), glm::vec3(100, 100, 100));
		skybox->draw(view, projection, lightDirection, cameraPosition);
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

#pragma region Rendering

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

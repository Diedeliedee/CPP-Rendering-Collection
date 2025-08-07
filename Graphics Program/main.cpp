#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"

#include "camera.h"
#include "skybox.h"
#include "terrain.h"
#include "model.h"
#include "portal.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace util;

//	Main:
int init(GLFWwindow*& window);

//	Rendering:
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

//	Util:
void createShaders();

//	Input:
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//	Window:
const int width = 1280, height = 720;

//	Program ID's:
GLuint modelProgram;

Camera*		camera;
Skybox*		skybox;
Terrain*	terrain;
Portal*		portal;
//Model*	backpack;

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

	//	Load resources.
	createShaders();

	//	Creating stuff for models.
	//backpack	= new Model("models/backpack/backpack.obj");

	//	Creating a portal.
	camera		= new Camera(width, height);
	skybox		= new Skybox();
	terrain		= new Terrain();
	portal		= new Portal(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), 100);

	//	Create a viewport.
	glViewport(0, 0, width, height);

	//	Game loop.
	while (!glfwWindowShouldClose(window))
	{
		//	Close window if escape is pressed.
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}

		//	Input.
		camera->processInput(window);

		//	Clearing previous draw.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//	Rendering models.
		//float t = glfwGetTime();
		//renderModel(backpack, glm::vec3(1000, 100, 1000), glm::vec3(0, 0, 0), glm::vec3(100, 100, 100));
		skybox->draw(camera->view,	camera->projection,	camera->cameraPosition);
		terrain->draw(camera->view,	camera->projection,	skybox->lightDirection, camera->cameraPosition);
		portal->draw();

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

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
	glfwSetCursorPosCallback(window,	mouse_callback);
	glfwSetKeyCallback(window,			key_callback);

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
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"),			1, GL_FALSE, glm::value_ptr(camera->view));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"),	1, GL_FALSE, glm::value_ptr(camera->projection));

	//	Passing world light information into the render program.
	glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(skybox->lightDirection));
	glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(camera->cameraPosition));

	//	Calling the model's render program.
	model->Draw(modelProgram);

	//	Disabling blending.
	glDisable(GL_BLEND);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera->mouseTick(xpos, ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	camera->keyTick(key, scancode, action);
}

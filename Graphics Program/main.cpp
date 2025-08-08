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
#include "object.h"
#include "portal.h"
#include "projection.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//	Fancy namespaces:
using namespace util;

//	Main:
int init(GLFWwindow*& window);

//	Rendering:
void switchToBuffer(unsigned int buffer);
void drawObjects(Projection* _projection);

//	Input:
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//	Window:
const int width = 1280, height = 720;

//	Objects:
Camera*		camera;
Skybox*		skybox;
Terrain*	terrain;
Portal*		portalA;
Portal*		portalB;

//	Framebuffer stuff
unsigned int portalBufA, portalColorBufA, portalDepthBufA;
unsigned int portalBufB, portalColorBufB, portalDepthBufB;

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

	//	Creating a portal.
	camera		= new Camera(width, height);
	skybox		= new Skybox();
	terrain		= new Terrain();
	portalA		= new Portal(camera, glm::vec3(1000, 500, 1000), 100);
	portalB		= new Portal(camera, glm::vec3(2000, 250, 2000), 100);

	//	Linking portals.
	portalA->linkedPortal = portalB;
	portalB->linkedPortal = portalA;

	//	Creating portal buffers.
	createFrameBuffer(width, height, portalBufA, portalColorBufA, portalDepthBufA);
	createFrameBuffer(width, height, portalBufB, portalColorBufB, portalDepthBufB);

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

		//	Drawing to PortalA buffer.
		switchToBuffer(portalBufA);
		portalA->tick();
		portalA->updatePortalProjection();
		portalB->enabled = false;
		drawObjects(portalA->portalProjection);
		portalB->enabled = true;

		//	Drawing to PortalB buffer.
		switchToBuffer(portalBufB);
		portalB->tick();
		portalB->updatePortalProjection();
		portalA->enabled = false;
		drawObjects(portalB->portalProjection);
		portalA->enabled = true;

		//	Back to main stuff.
		switchToBuffer(0);
		drawObjects(camera);

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

void switchToBuffer(unsigned int buffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, buffer);
	glViewport(0, 0, width, height);
}

/// <summary>
/// Draws every object in the scene.
/// </summary>
void drawObjects(Projection* _projection)
{
	//	Clearing previous draw.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	Drawing objects.
	skybox->		draw(_projection->view, _projection->projection, _projection->position);
	terrain->		draw(_projection->view, _projection->projection, skybox->lightDirection, _projection->position);
	portalA->		draw(_projection->view, _projection->projection, skybox->lightDirection, _projection->position, portalColorBufA);
	portalB->		draw(_projection->view, _projection->projection, skybox->lightDirection, _projection->position, portalColorBufB);
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
/// GLFW mouse callback to handle input.
/// </summary>
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera->mouseTick(xpos, ypos);
}

/// <summary>
/// GLFW key callback to handle input.
/// </summary>
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	camera->keyTick(key, scancode, action);
}

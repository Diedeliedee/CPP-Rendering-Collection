#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//	Forward declaration:
void processInput(GLFWwindow* window);

int main()
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
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Graphics Program", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "ERROR: Failed to create GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//	Load GLAD.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "ERROR: Failed to load Glad." << std::endl;
		return -1;
	}

	//	Create a viewport.
	glViewport(0, 0, 1280, 720);

	//	Game loop.
	while (!glfwWindowShouldClose(window))
	{
		//	Input.
		processInput(window);

		//	Rendering.
		glClearColor(0.25f, 0.5f, 0.75f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
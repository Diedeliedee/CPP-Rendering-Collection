#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//	Forward declaration:
int Setup(GLFWwindow*& window);			//	Try to find out what the difference is between (Class* Param), and (Class* &Param).
void processInput(GLFWwindow* window);
void CreateTriangle(GLuint &vao, int &size);
void CreateShader();
void loadFile(const char* filename, char*& output);

int main()
{
	//	Initialize the window.
	GLFWwindow* window = NULL;
	if (Setup(window) < 0) return -1;

	GLuint triangleVAO;
	int triangleSize;

	CreateTriangle(triangleVAO, triangleSize);
	CreateShader();

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

		glBindVertexArray(triangleVAO);
		glDrawArrays(GL_TRIANGLES, 0, triangleSize);

		//	Swap & Poll.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//	Close the application.
	glfwTerminate();
	return 0;
}

int Setup(GLFWwindow*& window)
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
	window = glfwCreateWindow(1280, 720, "Graphics Program", NULL, NULL);
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

	return 0;
}

void CreateTriangle(GLuint& vao, int& size)
{
	float vertices[] =
	{
		-0.5f,	-0.5f,	0.0f,
		0.5f,	-0.5f,	0.0f,
		0.0f,	0.5f,	0.0f,
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	size = sizeof(vertices);
}

void CreateShader()
{

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
		file.seekg(0, file.beg());

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

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
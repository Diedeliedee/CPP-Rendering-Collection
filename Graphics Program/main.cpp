#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//	Forward declaration:
int Setup(GLFWwindow*& window);			//	Try to find out what the difference is between (Class* Param), and (Class* &Param).
void processInput(GLFWwindow* window);
void CreateTriangle(GLuint& vao, int& size);
void CreateSquare(GLuint& VAO, GLuint& VBO, int& size, int& indices);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);

//	Util:
void loadFile(const char* filename, char*& output);

//	Program ID's
GLuint simpleProgram;

int main()
{
	//	Initialize the window.
	GLFWwindow* window = NULL;
	if (Setup(window) < 0) return -1;

	GLuint triangleVAO;
	GLuint triangleVBO;
	int triangleSize;
	int triangleIndexCount;

	createShaders();
	//CreateTriangle(triangleVAO, triangleSize);
	CreateSquare(triangleVAO, triangleVBO, triangleSize, triangleIndexCount);

	//	Create a viewport.
	glViewport(0, 0, 1280, 720);

	//	Game loop.
	while (!glfwWindowShouldClose(window))
	{
		//	Input.
		processInput(window);

		//	Rendering.
		glClearColor(0.6f, 0.45f, 0.67f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(simpleProgram);

		glBindVertexArray(triangleVAO);
		//glDrawArrays(GL_TRIANGLES, 0, triangleSize);
		glDrawElements(GL_TRIANGLES, triangleIndexCount, GL_UNSIGNED_INT, 0);

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

	//	Calculating the size.
	int stride	= 3 * sizeof(float);
	size		= sizeof(vertices) / stride;

	//	Creating the VAO index, and bingint it to create it's configuration.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//	Create buffer, bind it & assign vertices ot it.
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//	Set layout of vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void CreateSquare(GLuint& VAO, GLuint& EBO, int& size, int& numIndices)
{
	float vertices[] =
	{
		//	Position
		-0.5f,	-0.5f,	0.0f,	/*0*/	1.0f, 0.0f, 0.0f, 1.0f,	//	Red
		0.5f,	-0.5f,	0.0f,	/*1*/	0.0f, 1.0f, 0.0f, 1.0f,	//	Green
		-0.5f,	0.5f,	0.0f,	/*2*/	0.0f, 0.0f, 1.0f, 1.0f,	//	Blue
		0.5f,	0.5f,	0.0f,	/*3*/	1.0f, 1.0f, 1.0f, 1.0f	//	White
	};

	int indices[] =
	{
		0, 1, 2,
		2, 1, 3
	};

	//	Calculating the size and indices.
	int stride = (3 + 4) * sizeof(float);

	size		= sizeof(vertices) / stride;
	numIndices	= sizeof(indices) / sizeof(int);

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

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

}

void createShaders()
{
	createProgram(simpleProgram, "Shaders/simpleVertex.shader", "Shaders/simpleFragment.shader");
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

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}
#pragma once

#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"

class Portal
{
	GLuint program;

	GLuint VAO;
	GLuint EBO;
	int size;
	int numIndices;

public:
	Portal(glm::vec3 _position, glm::vec3 _rotation, float _scale)
	{
		util::createProgram(program, "shaders/portalVertex.shader", "shaders/terrainFragment.shader");
		createSquare(_scale, VAO, EBO, size, numIndices);
	}

	void draw()
	{
		//	Configuring options.
		glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//	Setting this program as the one to use.
		glUseProgram(program);

		//	Drawing!
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	}

private:
	/// <summary>
	/// Create the data for drawing the square,
	/// </summary>
	void createSquare(float _scale, GLuint& _VAO, GLuint& _EBO, int& _size, int& _numIndices)
	{
		float offset = 0.5 * _scale;

		float vertices[] =
		{
			-offset,	-offset,	0.0f,	//	0
			offset,		-offset,	0.0f,	//	1
			-offset,	offset,		0.0f,	//	2
			offset,		offset,		0.0f,	//	3
		};

		int indices[] =
		{
			0, 1, 2,
			2, 1, 3
		};

		//	Calculating the size and indices.
		int stride	= 3 * sizeof(float);
		_size		= sizeof(vertices) / stride;
		_numIndices = sizeof(indices) / sizeof(int);

		//	Creating the VAO index, and binding it to create it's configuration.
		glGenVertexArrays(1, &_VAO);
		glBindVertexArray(_VAO);

		//	Create buffer, bind it & assign vertices ot it.
		GLuint VBO;
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		//	Set layout of vertex data
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
};
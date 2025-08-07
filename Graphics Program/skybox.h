#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"

class Skybox
{
public:
	glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));

	Skybox()
	{
		//	Creating the shader.
		util::createProgram(program, "shaders/skyVertex.shader", "shaders/skyFragment.shader");

		//	Creating the box.
		createGeometry(boxVAO, boxEBO, boxSize, boxIndexCount);
	}

	void draw(glm::mat4 _view, glm::mat4 _projection, glm::vec3 _cameraPosition)
	{
		//	Configuring options.
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_DEPTH);

		//	Setting current program.
		glUseProgram(program);

		//	Creating world matrix.
		glm::mat4 world	= glm::mat4(1.0f);
		world			= glm::translate(world, _cameraPosition);
		world			= glm::scale(world, glm::vec3(100, 100, 100));

		//	Injecting projection data.
		glUniformMatrix4fv(glGetUniformLocation(program, "world"),		1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"),		1, GL_FALSE, glm::value_ptr(_view));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"),	1, GL_FALSE, glm::value_ptr(_projection));

		//	Injecting relevant vectors.
		glUniform3fv(glGetUniformLocation(program, "lightDirection"), 1, glm::value_ptr(lightDirection));
		glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1, glm::value_ptr(_cameraPosition));

		//	Drawing!
		glBindVertexArray(boxVAO);
		glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

		//	Re-enabling some stuff.
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH);
	}

private:
	GLuint program;

	GLuint boxVAO, boxEBO;
	int boxSize, boxIndexCount;

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
};
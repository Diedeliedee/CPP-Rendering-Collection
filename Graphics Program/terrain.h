#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"

class Terrain
{
public:
	Terrain()
	{
		//	Creating the terrain shader.
		util::createProgram(program, "shaders/terrainVertex.shader", "shaders/terrainFragment.shader");
		glUseProgram(program);
		glUniform1i(glGetUniformLocation(program, "diffuseTex"),	0);
		glUniform1i(glGetUniformLocation(program, "normalTex"),		1);
		glUniform1i(glGetUniformLocation(program, "dirt"),			2);
		glUniform1i(glGetUniformLocation(program, "sand"),			3);
		glUniform1i(glGetUniformLocation(program, "grass"),			4);
		glUniform1i(glGetUniformLocation(program, "rock"),			5);
		glUniform1i(glGetUniformLocation(program, "snow"),			6);

		//	Generating the plane.
		terrainVAO		= generatePlane("textures/heightmap.png", heightmapTexture, GL_RGBA, 4, 250.0f, 5.0f, terrainIndexCount, heightmapID);
		heightNormalID	= util::loadTexture("textures/heightnormal.png");

		dirt	= util::loadTexture("textures/dirt.jpg");
		sand	= util::loadTexture("textures/sand.jpg");
		grass	= util::loadTexture("textures/grass.png", 4);
		rock	= util::loadTexture("textures/rock.jpg");
		snow	= util::loadTexture("textures/snow.jpg");
	}

	void draw(glm::mat4 _view, glm::mat4 _projection, glm::vec3 _lightDirection, glm::vec3 _cameraPosition)
	{
		//	Configuring options.
		glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//	Setting current program.
		glUseProgram(program);

		//	Creating world matrix.
		glm::mat4 world = glm::mat4(1.0f);

		//	Injecting projection data.
		glUniformMatrix4fv(glGetUniformLocation(program, "world"),		1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"),		1, GL_FALSE, glm::value_ptr(_view));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"),	1, GL_FALSE, glm::value_ptr(_projection));

		//	Injecting relevant vectors.
		// float t = glfwGetTime();
		// lightDirection = glm::normalize(glm::vec3(glm::sin(t), -0.5f, glm::cos(t)));
		glUniform3fv(glGetUniformLocation(program, "lightDirection"), 1, glm::value_ptr(_lightDirection));
		glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1, glm::value_ptr(_cameraPosition));

		//	Injecting height textures.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmapID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, heightNormalID);

		//	Injecting ground textures.
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

		//	Drawing!
		glBindVertexArray(terrainVAO);
		glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);
	}

private:
	GLuint program;

	GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
	unsigned char* heightmapTexture;
	GLuint dirt, sand, grass, rock, snow;

	/// <summary>
	/// Function that creates a plane
	/// </summary>
	unsigned int generatePlane(const char* heightmap, unsigned char*& data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID)
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

		int stride = 8;
		float* vertices = new float[(width * height) * stride];
		unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];

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
};
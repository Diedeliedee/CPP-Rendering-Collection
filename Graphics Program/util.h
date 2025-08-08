#pragma once

#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "stb_image.h"

namespace util 
{
	/// <summary>
	/// Function to load a file from the computer's directory.
	/// </summary>
	/// <param name="filename">The path to pull the data from.</param>
	/// <param name="output">Character array defining the pulled date.</param>
	inline void loadFile(const char* filename, char*& output)
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

	/// <summary>
	/// Function to load a texture from the computers directory.
	/// </summary>
	/// <param name="path">The path to pull the texture from.</param>
	/// <param name="comp">Override for how many componenst the texture has. (Channels)</param>
	/// <returns>The texture!!</returns>
	inline GLuint loadTexture(const char* path, int comp = 0)
	{
		//	Generate and bind a texture. (Whatever that means ;_:)
		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		//	Setting texture parameters.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//	Loading the texture.
		int width, height, numChannels;
		unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);

		//	Setting data.
		if (data)
		{
			if (comp != 0) numChannels = comp;

			if (numChannels == 3)	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			else if (numChannels == 4)	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Error loading texture: " << path << "." << std::endl;
		}

		//	Unloading texture.
		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);

		//	Return it.
		return textureID;
	}

	/// <summary>
	/// Create a new program! (Comparable to Unity shader instance i.e material.)
	/// </summary>
	/// <param name="programID">Unique identifier for the program.</param>
	/// <param name="vertex">Paths to the vertex shader.</param>
	/// <param name="fragment">Path to the fragment shader.</param>
	inline void createProgram(GLuint& programID, const char* vertex, const char* fragment)
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

	inline void createFrameBuffer(int width, int height, unsigned int& frameBufferID, unsigned int& colorBufferID, unsigned int& depthBufferID)
	{
		//	Generate frame buffer.
		glGenFramebuffers(1, &frameBufferID);

		//	Generate color buffer.
		glGenTextures(1, &colorBufferID);
		glBindTexture(GL_TEXTURE_2D, colorBufferID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		//	Generate depth buffer.
		glGenRenderbuffers(1, &depthBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

		//	Attach buffers.
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferID, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBufferID);

		//	Check if succesful.
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Framebuffer not complete!" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};
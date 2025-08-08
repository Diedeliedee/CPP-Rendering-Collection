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
#include "model.h"

class Portal
{
public:
	glm::vec3 pos = glm::vec3(1, 1, 1), scale = glm::vec3(1, 1, 1);
	float diameter;

	Portal(glm::vec3 _position, float _scale)
	{
		pos			= _position;
		scale		= glm::vec3(_scale, _scale, _scale);
		diameter	= _scale;

		util::createProgram(program, "shaders/portalVertex.shader", "shaders/portalFragment.shader");

		sphere		= new Model("models/portal/portal.obj");
		testTexture	= util::loadTexture("textures/rock.jpg");
	}

	void draw(unsigned int& _renderTexture, glm::mat4 _view, glm::mat4 _projection, glm::vec3 _lightDirection, glm::vec3 _cameraPosition)
	{
		//	Configuring options.
		glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		//	Prioritizing program.
		glUseProgram(program);

		//	Passing translation data into the program.
		glm::mat4 world = glm::mat4(1.0f);

		world = glm::translate(world, pos);
		world = world * glm::toMat4(glm::quat(glm::vec3(0, 0, 0)));
		world = glm::scale(world, scale);

		glUniformMatrix4fv(glGetUniformLocation(program, "world"), 1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(_view));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

		//	Passing world light information into the render program.
		glUniform3fv(glGetUniformLocation(program, "lightDirection"), 1, glm::value_ptr(_lightDirection));
		glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1, glm::value_ptr(_cameraPosition));

		//	Passing the render texture into the shader.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _renderTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, testTexture);

		glUniform1i(glGetUniformLocation(program, "renderTexture"),	0);
		glUniform1i(glGetUniformLocation(program, "testTexture"),	1);

		//	Calling the model's render program.
		sphere->Draw(program);

		//	Disabling blending.
		glDisable(GL_BLEND);
	}

private:
	//	References:
	GLuint program;
	Model* sphere;

	//	Debug:
	GLuint testTexture;
};
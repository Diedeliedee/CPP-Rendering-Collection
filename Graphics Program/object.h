#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util.h"
#include "model.h"

class Object
{
public:

	Model* model;
	glm::vec3 pos, rot, scale;

	Object(string const& _path)
	{
		model	= new Model(_path);
		pos		= glm::vec3(0, 0, 0);
		rot		= glm::vec3(0, 0, 0);
		scale	= glm::vec3(1, 1, 1);

		setup();
	}

	Object(string const& _path, glm::vec3 _pos, glm::vec3 _rot, glm::vec3 _scale)
	{
		model	= new Model(_path);;
		pos		= _pos;
		rot		= _rot;
		scale	= _scale;

		setup();
	}

	void draw(glm::mat4 _view, glm::mat4 _projection, glm::vec3 _lightDirection, glm::vec3 _cameraPosition)
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

		glUseProgram(program);

		//	Passing translation data into the program.
		glm::mat4 world = glm::mat4(1.0f);

		world = glm::translate(world, pos);
		world = world * glm::toMat4(glm::quat(rot));
		world = glm::scale(world, scale);

		glUniformMatrix4fv(glGetUniformLocation(program, "world"), 1, GL_FALSE, glm::value_ptr(world));
		glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(_view));
		glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(_projection));

		//	Passing world light information into the render program.
		glUniform3fv(glGetUniformLocation(program, "lightDirection"), 1, glm::value_ptr(_lightDirection));
		glUniform3fv(glGetUniformLocation(program, "cameraPosition"), 1, glm::value_ptr(_cameraPosition));

		//	Calling the model's render program.
		model->Draw(program);

		//	Disabling blending.
		glDisable(GL_BLEND);
	}


private:
	GLuint program;

	void setup()
	{
		util::createProgram(program, "shaders/model.vs", "shaders/model.fs");
		glUseProgram(program);
		glUniform1i(glGetUniformLocation(program, "texture_diffuse1"), 0);
		glUniform1i(glGetUniformLocation(program, "texture_specular1"), 1);
		glUniform1i(glGetUniformLocation(program, "texture_norma1l"), 2);
		glUniform1i(glGetUniformLocation(program, "texture_roughness1"), 3);
		glUniform1i(glGetUniformLocation(program, "texture_ao1"), 4);
	}
};
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

class Projection
{
public:
	glm::vec3 position	= glm::vec3(0, 100, 0);
	float pitch			= 0;
	float yaw			= 0;
	int width			= 0;
	int height			= 0;

	glm::mat4 view, projection;

	Projection(int _width, int _height)
	{
		width	= _width;
		height	= _height;

		recalculate();
	}

	Projection(int _width, int _height, glm::vec3 _position, float _camPitch, float _camYaw)
	{
		position	= _position;
		pitch		= _camPitch;
		yaw			= _camYaw;

		recalculate();
	}

	void recalculate()
	{
		camQuat = glm::quat(glm::vec3(glm::radians(pitch), glm::radians(yaw), 0));

		glm::vec3 camForward	= camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp			= camQuat * glm::vec3(0, 1, 0);

		view		= glm::lookAt(position, position + camForward, camUp);
		projection	= glm::perspective(glm::radians(75.0f), width / (float)height, 0.1f, 5000.0f);
	}

protected:
	glm::quat camQuat;
};
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "projection.h"

class Camera : public Projection
{
public:

	Camera(int _width, int _height) : Projection(_width, _height)
	{
		
	}

	void processInput(GLFWwindow* window)
	{
		//	Define false cam changed.
		bool camChanged = false;

		//	Move the camera if a key is pressed!
		if (keys[GLFW_KEY_W])
		{
			position += camQuat * glm::vec3(0, 0, 5);
			camChanged = true;
		}
		if (keys[GLFW_KEY_S])
		{
			position += camQuat * glm::vec3(0, 0, -5);
			camChanged = true;
		}
		if (keys[GLFW_KEY_A])
		{
			position += camQuat * glm::vec3(5, 0, 0);
			camChanged = true;
		}
		if (keys[GLFW_KEY_D])
		{
			position += camQuat * glm::vec3(-5, 0, 0);
			camChanged = true;
		}

		//	If the camera has moved, recalculate the view matrix.
		if (camChanged)
		{
			recalculate();
		}
	}

	void mouseTick(double& _xpos, double& _ypos)
	{
		float x = (float)_xpos / 2;
		float y = (float)_ypos / 2;

		if (firstMouse)
		{
			lastX = x;
			lastY = y;
			firstMouse = false;
		}

		float dx	= x - lastX;
		float dy	= y - lastY;
		lastX		= x;
		lastY		= y;

		yaw		-= dx;
		pitch	= glm::clamp(pitch + dy, -90.0f, 90.0f);

		if (yaw > 180.0f)	yaw -= 360.0f;
		if (yaw < -180.0f)	yaw += 360.0f;

		recalculate();
	}

	void keyTick(int _key, int _scancode, int _action)
	{
		if (_action == GLFW_PRESS)
		{
			keys[_key] = true;
		}
		else if (_action == GLFW_RELEASE)
		{
			keys[_key] = false;
		}
	}

private:
	bool keys[1024] = { false };

	float lastX = 0, lastY = 0;
	bool firstMouse = true;
};
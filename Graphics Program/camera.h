#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

class Camera
{
public:
	glm::vec3 cameraPosition	= glm::vec3(0, 100, 0);
	glm::quat camQuat			= glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));
	glm::mat4 view, projection;

	Camera(int _width, int _height)
	{
		view		= glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		projection	= glm::perspective(glm::radians(75.0f), _width / (float)_height, 0.1f, 5000.0f);
	}

	void processInput(GLFWwindow* window)
	{
		//	Define false cam changed.
		bool camChanged = false;

		//	Move the camera if a key is pressed!
		if (keys[GLFW_KEY_W])
		{
			cameraPosition += camQuat * glm::vec3(0, 0, 10);
			camChanged = true;
		}
		if (keys[GLFW_KEY_S])
		{
			cameraPosition += camQuat * glm::vec3(0, 0, -10);
			camChanged = true;
		}
		if (keys[GLFW_KEY_A])
		{
			cameraPosition += camQuat * glm::vec3(10, 0, 0);
			camChanged = true;
		}
		if (keys[GLFW_KEY_D])
		{
			cameraPosition += camQuat * glm::vec3(-10, 0, 0);
			camChanged = true;
		}

		//	If the camera has moved, recalculate the view matrix.
		if (camChanged)
		{
			glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
			glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);

			view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
		}
	}

	void mouseTick(double& _xpos, double& _ypos)
	{
		float x = (float)_xpos;
		float y = (float)_ypos;

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

		camYaw		-= dx;
		camPitch	= glm::clamp(camPitch + dy, -90.0f, 90.0f);

		if (camYaw > 180.0f)	camYaw -= 360.0f;
		if (camYaw < -180.0f)	camYaw += 360.0f;

		camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

		glm::vec3 camForward	= camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp			= camQuat * glm::vec3(0, 1, 0);

		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
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

	float lastX = 0,	lastY = 0;
	float camPitch = 0,	camYaw = 0;

	bool firstMouse = true;
};
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
	//	Properties:
	glm::vec3 pos	= glm::vec3(1, 1, 1);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	float diameter	= 0;
	bool enabled	= true;

	//	Portal linking:
	Portal* linkedPortal			= NULL;
	Projection* portalProjection	= NULL;

	//	Portal logic:
	bool inPortal		= false;
	bool teleportedFlag = false;

	Portal(Projection* _mainCamera, glm::vec3 _position, float _scale)
	{
		baseProjection		= _mainCamera;
		portalProjection	= new Projection(baseProjection->width, baseProjection->height);

		pos			= _position;
		scale		= glm::vec3(_scale, _scale, _scale);
		diameter	= _scale;

		util::createProgram(program, "shaders/portalVertex.shader", "shaders/portalFragment.shader");

		sphere		= new Model("models/portal/portal.obj");
		testTexture	= util::loadTexture("textures/rock.jpg");
	}

	void tick()
	{
		glm::vec3 offset	= baseProjection->position - pos;
		float distance		= glm::length(offset);

		//	If the distance between the camera and portal is smaller than the portal radius
		if (distance < diameter / 2)
		{
			//	If the camera only just entered this frame.
			if (!inPortal)
			{
				//	If the other portal has teleported the camera here..
				if (linkedPortal->teleportedFlag)
				{
					//	Do nothing and deactivate the other portal's flag.
					inPortal						= true;
					linkedPortal->teleportedFlag	= false;
				}

				//	If we voluntarily entered..
				else
				{
					//	Then teleport us to the other portal.
					baseProjection->position = linkedPortal->pos + offset;
					baseProjection->recalculate();

					teleportedFlag = true;
				}
			}
		}
		else
		{
			inPortal = false;
		}
	}

	void updatePortalProjection()
	{
		//	Mimicing projection based on linked portal.
		if (linkedPortal != NULL)
		{
			//	Linked portal position + (base projection to this portal offset).
			portalProjection->position = linkedPortal->pos + (baseProjection->position - pos);
		}
		else
		{
			//	Just copy the position of the base projection.
			portalProjection->position = baseProjection->position;
		}

		//	Pitch and yaw always stays the same.
		portalProjection->pitch	= baseProjection->pitch;
		portalProjection->yaw	= baseProjection->yaw;
		portalProjection->recalculate();
	}

	void draw(glm::mat4 _view, glm::mat4 _projection, glm::vec3 _lightDirection, glm::vec3 _cameraPosition, unsigned int& _renderTexture)
	{
		if (!enabled) return;

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

		//	Create a variable for the portal view.
		unsigned int portalTexture = 0;

		//	If there's not linked portal, display a test texture through the portal instead.
		if (linkedPortal != NULL)	portalTexture = _renderTexture;
		else						portalTexture = testTexture;

		//	Bind and pass the portal texture.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, portalTexture);
		glUniform1i(glGetUniformLocation(program, "renderTexture"),	0);

		//	Calling the model's render program.
		sphere->Draw(program);

		//	Disabling blending.
		glDisable(GL_BLEND);
	}

private:
	//	Shader:
	GLuint program;

	//	Model:
	Model* sphere = NULL;

	//	Camera view:
	Projection* baseProjection = NULL;

	//	Debug:
	GLuint testTexture;

};
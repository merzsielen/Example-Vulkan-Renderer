/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Camera.cpp																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

namespace VkExample
{
	/*---------------------------------------------------------------------------------------------*/
	/* Camera																					   */
	/*---------------------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* View & Projection Functions											 */
	/*-----------------------------------------------------------------------*/
	/* Update Projection ----------------------------------------------------*/
	void Camera::UpdateProjection()
	{
		float halfWidth = (viewport.width / 2.0f) * zoom;
		float halfHeight = (viewport.height / 2.0f) * zoom;
		projection = glm::ortho(-halfWidth, halfWidth, -halfWidth, halfHeight, nearClip, farClip);
	}

	/* Update View ----------------------------------------------------------*/
	void Camera::UpdateView()
	{
		glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f) * rotation;
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f) * rotation;
		view = glm::lookAt(position, position + forward, up);
	}

	/*-----------------------------------------------------------------------*/
	/* Constructor															 */
	/*-----------------------------------------------------------------------*/
	Camera::Camera(glm::vec3 position, glm::quat rotation, float zoom, float nearClip, float farClip)
	{
		this->position = position;
		this->rotation = rotation;
		this->zoom = zoom;
		this->nearClip = nearClip;
		this->farClip = farClip;

		this->viewport = {};
		this->scissor = {};

		this->view = glm::mat4(0);
		this->projection = glm::mat4(0);
	}
}
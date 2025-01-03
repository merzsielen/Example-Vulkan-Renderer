#ifndef POLYGONS_H
#define POLYGONS_H

/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Polygons.h																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

namespace VkExample
{
	/*-----------------------------------------------------------------------*/
	/* Vertices																 */
	/*-----------------------------------------------------------------------*/
	struct Vertex
	{
		glm::vec3	position;
		glm::vec4	color;
		glm::vec2	uv;

		/* Binding Description ------------------------------------*/
		static VkVertexInputBindingDescription BindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		/* Attribute Description ----------------------------------*/
		static std::vector<VkVertexInputAttributeDescription> AttributeDescriptions()
		{
			VkVertexInputAttributeDescription attDesc0{};
			attDesc0.binding = 0;
			attDesc0.location = 0;
			attDesc0.format = VK_FORMAT_R32G32B32_SFLOAT;
			attDesc0.offset = offsetof(Vertex, position);

			VkVertexInputAttributeDescription attDesc1{};
			attDesc1.binding = 0;
			attDesc1.location = 1;
			attDesc1.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attDesc1.offset = offsetof(Vertex, color);

			VkVertexInputAttributeDescription attDesc2{};
			attDesc2.binding = 0;
			attDesc2.location = 2;
			attDesc2.format = VK_FORMAT_R32G32_SFLOAT;
			attDesc2.offset = offsetof(Vertex, uv);

			return { attDesc0, attDesc1, attDesc2 };
		}
	};

	/*-----------------------------------------------------------------------*/
	/* Triangles															 */
	/*-----------------------------------------------------------------------*/
	struct Triangle
	{
		Vertex a, b, c;
	};

	/*-----------------------------------------------------------------------*/
	/* Quad																	 */
	/*-----------------------------------------------------------------------*/
	struct Quad
	{
		Triangle a, b;
	};
}

#endif
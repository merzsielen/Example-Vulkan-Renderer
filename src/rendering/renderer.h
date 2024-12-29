#ifndef RENDERER_H
#define RENDERER_H

/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Renderer.h																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>

#include "shader.h"
#include "../util/gpuhandler.h"
#include "../util/geometry.h"

#define MAX_TRIANGLES 100000
#define MAX_FRAMES_IN_FLIGHT 2

namespace Untitled
{
	/*---------------------------------------------------------------------------------------------*/
	/* Renderer																					   */
	/*---------------------------------------------------------------------------------------------*/
	class Renderer
	{
	private:
		/*-------------------------------------------------------------------*/
		/* Frame															 */
		/*-------------------------------------------------------------------*/
		unsigned int								frame;

		/*-------------------------------------------------------------------*/
		/* GPU Handler & Convenient Structs									 */
		/*-------------------------------------------------------------------*/
		GPUHandler*									gpuHandler;
		VkDevice									device;
		Swapchain									swapchain;
		VkExtent2D									extent;
		VkQueue										graphicsQueue;
		VkQueue										presentQueue;

		/*-------------------------------------------------------------------*/
		/* Vertex Buffer													 */
		/*-------------------------------------------------------------------*/
		VkBuffer									vertexBuffer;
		VkDeviceMemory								vertexBufferMemory;
		Vertex										vertices[MAX_TRIANGLES * 3];

		/*-------------------------------------------------------------------*/
		/* Shader															 */
		/*-------------------------------------------------------------------*/
		std::unordered_map<std::string, Shader>		shaders;

		/*-------------------------------------------------------------------*/
		/* Viewport & Scissor												 */
		/*-------------------------------------------------------------------*/
		VkViewport									viewport;
		VkRect2D									scissor;

		/*-------------------------------------------------------------------*/
		/* Render Pass														 */
		/*-------------------------------------------------------------------*/
		VkRenderPass								renderPass;

		/*-------------------------------------------------------------------*/
		/* Pipeline															 */
		/*-------------------------------------------------------------------*/
		VkPipeline									graphicsPipeline;
		VkPipelineLayout							pipelineLayout;

		/*-------------------------------------------------------------------*/
		/* Frame Buffers													 */
		/*-------------------------------------------------------------------*/
		std::vector<VkFramebuffer>					framebuffers;

		/*-------------------------------------------------------------------*/
		/* Command Pool & Buffer											 */
		/*-------------------------------------------------------------------*/
		VkCommandPool								commandPool;
		std::vector<VkCommandBuffer>				commandBuffers;

		/*-------------------------------------------------------------------*/
		/* Synchronization Objects											 */
		/*-------------------------------------------------------------------*/
		std::vector<VkSemaphore>					imagesAvailable;
		std::vector<VkSemaphore>					rendersFinished;
		std::vector<VkFence>						inFlights;

		/*-------------------------------------------------------------------*/
		/* Setup Functions													 */
		/*-------------------------------------------------------------------*/
		void										SetupFramebuffers();
		void										SetupRenderPasses();
		void										SetupPipeline(std::vector<VkDynamicState> dynamicStates, Shader baseShader);
		unsigned int								FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void										CreateBuffer(unsigned int size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void										SetupVertexBuffer();
		void										SetupCommands();
		void										SetupSynchronization();

	public:
		/*-------------------------------------------------------------------*/
		/* Render Functions													 */
		/*-------------------------------------------------------------------*/
		void										Render();

		/*-------------------------------------------------------------------*/
		/* Command Functions												 */
		/*-------------------------------------------------------------------*/
		void										RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		/*-------------------------------------------------------------------*/
		/* Repair Swapchain													 */
		/*-------------------------------------------------------------------*/
		void										RepairSwapchain();

		/*-------------------------------------------------------------------*/
		/* Constructor														 */
		/*-------------------------------------------------------------------*/
		Renderer(std::vector<VkDynamicState> dynamicStates, GPUHandler* gpuHandler);

		/*-------------------------------------------------------------------*/
		/* Deconstructor													 */
		/*-------------------------------------------------------------------*/
		~Renderer();
	};
}

#endif
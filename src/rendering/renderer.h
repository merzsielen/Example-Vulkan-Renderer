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
#include <set>
#include <vector>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <unordered_map>

#include "../util/polygons.h"
#include "camera.h"
#include "shader.h"

/*-------------------------------------------------------------------------------------------------*/
/* Defines																						   */
/*-------------------------------------------------------------------------------------------------*/

#define MAX_TRIANGLES 1000000
#define MAX_FRAMES_IN_FLIGHT 4
#define ENABLE_VALIDATION_LAYERS 1

namespace VkExample
{
	/*---------------------------------------------------------------------------------------------*/
	/* Helper Structs																			   */
	/*---------------------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* Queue Family Indices													 */
	/*-----------------------------------------------------------------------*/
	/*
		Queue family indices tell us about the queue families of a particular
		physical device.
	*/
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool IsComplete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	/*-----------------------------------------------------------------------*/
	/* Swap Chain Support Details											 */
	/*-----------------------------------------------------------------------*/
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/*-----------------------------------------------------------------------*/
	/* Swap Chain 															 */
	/*-----------------------------------------------------------------------*/
	struct SwapChain
	{
		VkSwapchainKHR base;
		std::vector<VkImage> images;
		VkFormat format;
		VkExtent2D extent;
		std::vector<VkImageView> imageViews;
	};

	/*-----------------------------------------------------------------------*/
	/* Uniform Buffer Object 												 */
	/*-----------------------------------------------------------------------*/
	struct UniformBufferObject
	{
		glm::mat4 mvp;
		glm::vec2 atlasDimens;
	};

	/*---------------------------------------------------------------------------------------------*/
	/* Renderer																					   */
	/*---------------------------------------------------------------------------------------------*/
	class Renderer
	{
	private:
		/*-------------------------------------------------------------------*/
		/* Frame															 */
		/*-------------------------------------------------------------------*/
		unsigned int					frame;

		/*-------------------------------------------------------------------*/
		/* GLFW																 */
		/*-------------------------------------------------------------------*/
		GLFWwindow*						window;
		bool							windowResized;

		/*-------------------------------------------------------------------*/
		/* Camera															 */
		/*-------------------------------------------------------------------*/
		Camera* camera;

		/*-------------------------------------------------------------------*/
		/* Vulkan															 */
		/*-------------------------------------------------------------------*/
		VkInstance						instance;
		VkSurfaceKHR					surface;

		VkPhysicalDevice				physicalDevice;
		VkDevice						device;

		QueueFamilyIndices				indices;

		SwapChain						swapChain;

		VkQueue							graphicsQueue;
		VkQueue							presentQueue;

		VkRenderPass					renderPass;

		VkDescriptorSetLayout			descriptorSetLayout;

		VkPipeline						graphicsPipeline;
		VkPipelineLayout				pipelineLayout;

		std::vector<VkFramebuffer>		framebuffers;

		VkCommandPool					commandPool;
		std::vector<VkCommandBuffer>	commandBuffers;

		/*-------------------------------------------------------------------*/
		/* Buffers															 */
		/*-------------------------------------------------------------------*/
		VkBuffer						stagingBuffer;
		VkDeviceMemory					stagingBufferMemory;

		VkBuffer						vertexBuffer;
		VkDeviceMemory					vertexBufferMemory;

		std::vector<VkBuffer>			uniformBuffers;
		std::vector<VkDeviceMemory>		uniformBuffersMemory;
		std::vector<void*>				uniformBuffersMapped;

		/*-------------------------------------------------------------------*/
		/* Synchronization Objects											 */
		/*-------------------------------------------------------------------*/
		std::vector<VkSemaphore>		imagesAvailable;
		std::vector<VkSemaphore>		rendersFinished;
		std::vector<VkFence>			inFlights;

		/*-------------------------------------------------------------------*/
		/* Shaders															 */
		/*-------------------------------------------------------------------*/
		std::unordered_map<std::string, Shader>	shaders;

		/*-------------------------------------------------------------------*/
		/* Vulkan Setup Functions											 */
		/*-------------------------------------------------------------------*/
		/* Validation Layers ------------------------------------------------*/
		bool							CheckValidationLayerSupport(std::vector<const char*> validationLayers);

		/* Instance Setup ---------------------------------------------------*/
		std::vector<const char*>		GetRequiredExtensions();
		void							CreateInstance(	std::string name,
														std::vector<const char*> validationLayers,
														std::vector<const char*> instanceExtensions);

		/* Surface Setup ----------------------------------------------------*/
		void							CreateSurface();

		/* Device Setup -----------------------------------------------------*/
		QueueFamilyIndices				FindQueueFamilies(VkPhysicalDevice potentiate);
		SwapChainSupportDetails			GetSwapChainSupportDetails(VkPhysicalDevice potentiate);
		bool							CheckDeviceExtensionSupport(VkPhysicalDevice potentiate,
																	std::vector<const char*> deviceExtensions);
		bool							CheckPhysicalDeviceSuitability(	VkPhysicalDevice potentiate,
																		std::vector<const char*> deviceExtensions);
		int								RatePhysicalDevice(VkPhysicalDevice potentiate);
		void							GetPhysicalDevice(std::vector<const char*> deviceExtensions);
		void							CreateDevice(	std::vector<const char*> validationLayers,
														std::vector<const char*> deviceExtensions);
		void							GetGraphicsQueue();
		void							GetPresentQueue();

		/* SwapChain Setup --------------------------------------------------*/
		VkSurfaceFormatKHR				ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR				ChooseSwapPresentMode(std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D						ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
		void							CreateSwapChain();

		/* Framebuffers Setup -----------------------------------------------*/
		void							SetupFramebuffers();

		/* Render Passes Setup ----------------------------------------------*/
		void							SetupRenderPasses();

		/* Descriptor Layout Setup ------------------------------------------*/
		void							SetupDescriptorLayout();

		/* Pipeline Setup ---------------------------------------------------*/
		void							SetupPipeline(std::vector<VkDynamicState> dynamicStates, Shader baseShader);

		/* Buffer Setup -----------------------------------------------------*/
		unsigned int					FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		void							CopyBuffer(VkBuffer src, VkBuffer dst, unsigned int size);
		void							CreateBuffer(unsigned int size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void							SetupVertexBuffer();
		void							SetupUniformBuffers();

		/* Commands Setup ---------------------------------------------------*/
		void							SetupCommands();

		/* Synchronization Setup --------------------------------------------*/
		void							SetupSynchronization();

		/*-------------------------------------------------------------------*/
		/* Command Functions												 */
		/*-------------------------------------------------------------------*/
		void							RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	public:
		/*-------------------------------------------------------------------*/
		/* Render Function													 */
		/*-------------------------------------------------------------------*/
		void							Render();

		/*-------------------------------------------------------------------*/
		/* Buffer Functions													 */
		/*-------------------------------------------------------------------*/
		void							WriteVertices(Vertex* vertices, unsigned int nVertices);
		void							WriteVertexBuffer(Vertex* vertices, unsigned int nVertices);
		void							WriteUniformBuffer(uint32_t imageIndex);

		/*-------------------------------------------------------------------*/
		/* Window Functions													 */
		/*-------------------------------------------------------------------*/
		void							SetWindowResized(bool v) { windowResized = v; }
		GLFWwindow*						GetWindow() { return window; }

		/*-------------------------------------------------------------------*/
		/* Constructor														 */
		/*-------------------------------------------------------------------*/
		Renderer(std::vector<VkDynamicState> dynamicStates, int screenWidth, int screenHeight, const char* title, Camera* camera);

		/*-------------------------------------------------------------------*/
		/* Deconstructor													 */
		/*-------------------------------------------------------------------*/
		~Renderer();
	};

	/*-----------------------------------------------------------------------*/
	/* GLFW Setup															 */
	/*-----------------------------------------------------------------------*/
	/*---------------------------------------------------*/
	/* Resize Callback									 */
	/*---------------------------------------------------*/
	static void ResizeCallback(GLFWwindow* window, int width, int height)
	{
		Renderer* r = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		r->SetWindowResized(true);
	}
}

#endif
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

#include "shader.h"
#include "../util/geometry.h"

#define MAX_TRIANGLES 2000000
#define MAX_FRAMES_IN_FLIGHT 4

namespace Untitled
{
	/*-------------------------------------------------------------------------------------------------*/
	/* Vulkan Setup																					   */
	/*-------------------------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------*/
	/* Extensions & Validation Layer Functions									 */
	/*---------------------------------------------------------------------------*/
	/* Check Validation Layer Support -------------------------------------------*/
	/*
		This function serves to check if the validation layers we are requesting
		are available. This is more important in the debug phase and should be
		disabled (by setting ENABLE_VALIDATION_LAYERS to 0) for release.
	*/
	bool CheckValidationLayerSupport(std::vector<const char*> validationLayers);

	/* Get Required Extensions --------------------------------------------------*/
	/*
		This function just gets the extensions from Vulkan which are required
		for GLFW (and anything else we might add later.
	*/
	std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);

	/*---------------------------------------------------------------------------*/
	/* Queue Family																 */
	/*---------------------------------------------------------------------------*/
	/* Queue Family Indices -----------------------------------------------------*/
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

	/* Find Queue Families ------------------------------------------------------*/
	/*
		Find Queue Families returns the queue family indices of a physical
		device.
	*/
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

	/*---------------------------------------------------------------------------*/
	/* Swap Chain Support														 */
	/*---------------------------------------------------------------------------*/
	/* Swap Chain Support Details -----------------------------------------------*/
	/*
		SCSD gives us data about the swap chain capabilities of a particular
		device.
	*/
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/* Get Swap Chain Support Details -------------------------------------------*/
	/*
		This returns to SCSDs of a particular device.
	*/
	SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

	/*---------------------------------------------------------------------------*/
	/* Surface Functions														 */
	/*---------------------------------------------------------------------------*/
	/* Create Surface -----------------------------------------------------------*/
	/*
		We have to create a surface for our window.
	*/
	VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window);

	/*---------------------------------------------------------------------------*/
	/* Physical Device Functions												 */
	/*---------------------------------------------------------------------------*/
	/* Check Device Extension Support -------------------------------------------*/
	/*
		This just checks that a given device has all the required extensions.
	*/
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions);

	/* Check Device Suitability -------------------------------------------------*/
	/*
		We need to check if a particular device is suitable for our purposes.
	*/
	bool CheckPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions);

	/* Rate Physical Device -----------------------------------------------------*/
	/*
		For now, we just get the best device we can. Perhaps in the future we
		should change this.
	*/
	int RatePhysicalDevice(VkPhysicalDevice device);

	/* Get Physical Device ------------------------------------------------------*/
	/*
		We need to find the proper physical device that supports Vulkan.
	*/
	VkPhysicalDevice GetPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions);

	/*---------------------------------------------------------------------------*/
	/* Logical Device Functions													 */
	/*---------------------------------------------------------------------------*/
	/* Create Logical Device ----------------------------------------------------*/
	/*
		In order to use our physical device we must create a corresponding
		logical device.
	*/
	VkDevice CreateLogicalDevice(VkPhysicalDevice device, QueueFamilyIndices indices, std::vector<const char*> validationLayers, std::vector<const char*> requiredExtensions, bool enableValidationLayers);

	/* Get Device Graphics Queue ---------------------------------------------------------*/
	/*
		Returns the graphics queue of the input device.
	*/
	VkQueue GetDeviceGraphicsQueue(VkDevice device, QueueFamilyIndices indices);

	/* Get Device Present Queue ---------------------------------------------------------*/
	/*
		Returns the present queue of the input device.
	*/
	VkQueue GetDevicePresentQueue(VkDevice device, QueueFamilyIndices indices);

	/*---------------------------------------------------------------------------*/
	/* Swap Chain 																 */
	/*---------------------------------------------------------------------------*/
	/* Swapchain ----------------------------------------------------------------*/
	struct Swapchain
	{
		VkSwapchainKHR base;
		std::vector<VkImage> images;
		VkFormat format;
		VkExtent2D extent;
		std::vector<VkImageView> imageViews;
	};

	/* Choose Swap Surface Format -----------------------------------------------*/
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats);

	/* Choose Swap Present Mode -------------------------------------------------*/
	VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR>& availableModes);

	/* Choose Swap Extent -------------------------------------------------------*/
	VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

	/* Create Swap Chain --------------------------------------------------------*/
	Swapchain CreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, GLFWwindow* window);

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

		/*---------------------------------------------------------------------------*/
		/* GLFW																		 */
		/*---------------------------------------------------------------------------*/
		GLFWwindow*									window;
		bool										framebufferResized;

		/*---------------------------------------------------------------------------*/
		/* Vulkan																	 */
		/*---------------------------------------------------------------------------*/
		VkInstance									instance;
		VkSurfaceKHR								surface;

		VkPhysicalDevice							physicalDevice;
		VkDevice									logicalDevice;

		Swapchain									swapchain;

		VkQueue										graphicsQueue;
		VkQueue										presentQueue;

		/*-------------------------------------------------------------------*/
		/* Buffers															 */
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
		void										CopyBuffer(VkBuffer src, VkBuffer dst, unsigned int size);
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
		void										ResizeFramebuffer(bool t) { framebufferResized = t; }
		void										RepairSwapchain();

		/*-------------------------------------------------------------------*/
		/* Gets																 */
		/*-------------------------------------------------------------------*/
		GLFWwindow*									GetWindow() { return window; }

		/*-------------------------------------------------------------------*/
		/* Constructor														 */
		/*-------------------------------------------------------------------*/
		Renderer(std::vector<VkDynamicState> dynamicStates, int screenWidth, int screenHeight, const char* title, bool enableValidationLayers);

		/*-------------------------------------------------------------------*/
		/* Deconstructor													 */
		/*-------------------------------------------------------------------*/
		~Renderer();
	};

	/*-------------------------------------------------------------------------------------------------*/
	/* GLFW Setup																					   */
	/*-------------------------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------*/
	/* Resize Callback															 */
	/*---------------------------------------------------------------------------*/
	static void ResizeCallback(GLFWwindow* window, int width, int height)
	{
		Renderer* r = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		r->ResizeFramebuffer(true);
	}
}

#endif
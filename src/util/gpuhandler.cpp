/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* GPUHandler.cpp																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* General																						   */
/*-------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Includes																	 */
/*---------------------------------------------------------------------------*/
#include "gpuhandler.h"

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
	bool CheckValidationLayerSupport(std::vector<const char*> validationLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (int i = 0; i < validationLayers.size(); i++)
		{
			bool layerFound = false;

			for (int j = 0; j < availableLayers.size(); j++)
			{
				if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound) return false;
		}

		return true;
	}

	/* Get Required Extensions --------------------------------------------------*/
	/*
		This function just gets the extensions from Vulkan which are required
		for GLFW (and anything else we might add later.
	*/
	std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers)
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}

	/*---------------------------------------------------------------------------*/
	/* Queue Family																 */
	/*---------------------------------------------------------------------------*/
	/* Find Queue Families ------------------------------------------------------*/
	/*
		Find Queue Families returns the queue family indices of a physical
		device.
	*/
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (int j = 0; j < queueFamilies.size(); j++)
		{
			VkQueueFamilyProperties queueFamily = queueFamilies[j];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport) indices.presentFamily = i;
			if (indices.IsComplete()) break;
			i++;
		}

		return indices;
	}

	/*---------------------------------------------------------------------------*/
	/* Swap Chain Support														 */
	/*---------------------------------------------------------------------------*/
	/* Get Swap Chain Support Details -------------------------------------------*/
	/*
		This returns to SCSDs of a particular device.
	*/
	SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	/*---------------------------------------------------------------------------*/
	/* Surface Functions														 */
	/*---------------------------------------------------------------------------*/
	/* Create Surface -----------------------------------------------------------*/
	/*
		We have to create a surface for our window.
	*/
	VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window)
	{
		VkSurfaceKHR surface;
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface.");
		}
		return surface;
	}

	/*---------------------------------------------------------------------------*/
	/* Physical Device Functions												 */
	/*---------------------------------------------------------------------------*/
	/* Check Device Extension Support -------------------------------------------*/
	/*
		This just checks that a given device has all the required extensions.
	*/
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> requiredExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		for (int i = 0; i < requiredExtensions.size(); i++)
		{
			bool extensionFound = false;

			for (int j = 0; j < availableExtensions.size(); j++)
			{
				if (strcmp(requiredExtensions[i], availableExtensions[j].extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound) return false;
		}

		return true;
	}

	/* Check Device Suitability -------------------------------------------------*/
	/*
		We need to check if a particular device is suitable for our purposes.
	*/
	bool CheckPhysicalDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions)
	{
		QueueFamilyIndices indices = FindQueueFamilies(device, surface);

		if (!CheckDeviceExtensionSupport(device, requiredExtensions)) return false;

		SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(device, surface);
		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) return false;

		return (indices.IsComplete());
	}

	/* Rate Physical Device -----------------------------------------------------*/
	/*
		For now, we just get the best device we can. Perhaps in the future we
		should change this.
	*/
	int RatePhysicalDevice(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (!deviceFeatures.geometryShader) return 0;

		int score = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
		score += deviceProperties.limits.maxImageDimension2D;
		return score;
	}

	/* Get Physical Device ------------------------------------------------------*/
	/*
		We need to find the proper physical device that supports Vulkan.
	*/
	VkPhysicalDevice GetPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::vector<const char*> requiredExtensions)
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find GPUs with Vulkan support.");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		int highestScore = 0;
		VkPhysicalDevice bestDevice = VK_NULL_HANDLE;

		for (int i = 0; i < devices.size(); i++)
		{
			VkPhysicalDevice device = devices[i];
			if (CheckPhysicalDeviceSuitability(device, surface, requiredExtensions))
			{
				int score = RatePhysicalDevice(device);

				if (score > highestScore)
				{
					bestDevice = device;
					highestScore = score;
				}
			}
		}

		// If the device is still null, that means we found no suitable GPU.
		if (bestDevice == VK_NULL_HANDLE) throw std::runtime_error("Unable to find suitable GPU.");

		return bestDevice;
	}

	/*---------------------------------------------------------------------------*/
	/* Logical Device Functions													 */
	/*---------------------------------------------------------------------------*/
	/* Create Logical Device ----------------------------------------------------*/
	/*
		In order to use our physical device we must create a corresponding
		logical device.
	*/
	VkDevice CreateLogicalDevice(VkPhysicalDevice device, QueueFamilyIndices indices, std::vector<const char*> validationLayers, std::vector<const char*> requiredExtensions, bool enableValidationLayers)
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (std::set<uint32_t>::iterator it = uniqueQueueFamilies.begin(); it != uniqueQueueFamilies.end(); it++)
		{
			uint32_t queueFamily = *it;
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.logicOp = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = queueCreateInfos.size();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		createInfo.enabledLayerCount = 0;

		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		VkDevice logicalDevice = VK_NULL_HANDLE;
		if (vkCreateDevice(device, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device.");
		}

		return logicalDevice;
	}

	/* Get Device Graphics Queue ---------------------------------------------------------*/
	/*
		Returns the graphics queue of the input device.
	*/
	VkQueue GetDeviceGraphicsQueue(VkDevice device, QueueFamilyIndices indices)
	{
		VkQueue queue;
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &queue);
		return queue;
	}

	/* Get Device Present Queue ---------------------------------------------------------*/
	/*
		Returns the present queue of the input device.
	*/
	VkQueue GetDevicePresentQueue(VkDevice device, QueueFamilyIndices indices)
	{
		VkQueue queue;
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &queue);
		return queue;
	}

	/*---------------------------------------------------------------------------*/
	/* Swap Chain 																 */
	/*---------------------------------------------------------------------------*/
	/* Choose Swap Surface Format -----------------------------------------------*/
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (int i = 0; i < availableFormats.size(); i++)
		{
			if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormats[i];
			}
		}

		return availableFormats[0];
	}

	/* Choose Swap Present Mode -------------------------------------------------*/
	VkPresentModeKHR ChooseSwapPresentMode(std::vector<VkPresentModeKHR>& availableModes)
	{
		for (int i = 0; i < availableModes.size(); i++)
		{
			if (availableModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) return availableModes[i];
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Choose Swap Extent -------------------------------------------------------*/
	VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { width, height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	/* Create Swap Chain --------------------------------------------------------*/
	Swapchain CreateSwapChain(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, GLFWwindow* window)
	{
		SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(physicalDevice, surface);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, window);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR swapchainKHR;

		if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchainKHR) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain.");
		}

		std::vector<VkImage> swapchainImages;
		vkGetSwapchainImagesKHR(logicalDevice, swapchainKHR, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(logicalDevice, swapchainKHR, &imageCount, swapchainImages.data());

		Swapchain swapchain = { swapchainKHR, swapchainImages, surfaceFormat.format, extent, {} };

		swapchain.imageViews.resize(swapchainImages.size());

		for (int i = 0; i < swapchainImages.size(); i++)
		{
			VkImageViewCreateInfo imgCreateInfo{};
			imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgCreateInfo.image = swapchainImages[i];
			imgCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imgCreateInfo.format = swapchain.format;
			imgCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgCreateInfo.subresourceRange.baseMipLevel = 0;
			imgCreateInfo.subresourceRange.levelCount = 1;
			imgCreateInfo.subresourceRange.baseArrayLayer = 0;
			imgCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(logicalDevice, &imgCreateInfo, nullptr, &swapchain.imageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image views.");
			}
		}

		return swapchain;
	}

	/*-------------------------------------------------------------------------------------------------*/
	/* Graphics Handler																				   */
	/*-------------------------------------------------------------------------------------------------*/
	/*---------------------------------------------------------------------------*/
	/* Repair Swapchain															 */
	/*---------------------------------------------------------------------------*/
	void GPUHandler::RepairSwapchain()
	{
		swapchain = CreateSwapChain(physicalDevice, logicalDevice, surface, window);
	}

	/*---------------------------------------------------------------------------*/
	/* Constructor																 */
	/*---------------------------------------------------------------------------*/
	GPUHandler::GPUHandler(int screenWidth, int screenHeight, const char* title, bool enableValidationLayers)
	{
		/*-----------------------------------------------------------------------*/
		/* GLFW Setup					        								 */
		/*-----------------------------------------------------------------------*/
		this->framebufferResized = false;
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(screenWidth, screenHeight, "Untitled", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, ResizeCallback);

		/*-----------------------------------------------------------------------*/
		/* Vulkan Setup					        								 */
		/*-----------------------------------------------------------------------*/
		/*
			Before anything else, we need to set up our error-checking for Vulkan
			because it doesn't play nice with us like OpenGL does.
		*/
		std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers))
		{
			throw std::runtime_error("Some of the requested validation layers are not available.");
		}

		/*
			We also need to ensure that certain device extensions are present.
		*/
		std::vector<const char*> deviceExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME
		};

		std::vector<const char*> instanceExtensions
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
		};


		/*
			Now, we pass some information about our application. This includes
			some basic stuff like its name and engine and whatnot.
		*/
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Untitled";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Untitled Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		/*
			Next, we pass some more information about the instance we would like
			to create.
		*/
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		/*
			We need to get some info from GLFW in order to make it work nicely
			with Vulkan.
		*/
		std::vector<const char*> glfwExtensions = GetRequiredExtensions(enableValidationLayers);
		for (int i = 0; i < glfwExtensions.size(); i++) instanceExtensions.push_back(glfwExtensions[i]);
		createInfo.enabledExtensionCount = instanceExtensions.size();
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		/*
			And now we create our instance.
		*/
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		/*
			We've gotta create a surface now. I feel like we have to create
			a million little things for Vulkan to work.
		*/
		surface = CreateSurface(instance, window);

		/*
			But of course we're not done yet. We need to select our GPU.
		*/
		physicalDevice = GetPhysicalDevice(instance, surface, deviceExtensions);
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		/*
			We have to create our logical device now. Because of course this
			can't be easy. We have chosen the way of pain.
		*/
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
		logicalDevice = CreateLogicalDevice(physicalDevice, indices, validationLayers, deviceExtensions, enableValidationLayers);
		graphicsQueue = GetDeviceGraphicsQueue(logicalDevice, indices);
		presentQueue = GetDevicePresentQueue(logicalDevice, indices);

		std::cout << "Using the GPU: " << physicalDeviceProperties.deviceName << std::endl;

		/*
			The torment never ends. We now have to make our swap chain.
		*/
		swapchain = CreateSwapChain(physicalDevice, logicalDevice, surface, window);
	}

	/*---------------------------------------------------------------------------*/
	/* Deconstructor															 */
	/*---------------------------------------------------------------------------*/
	GPUHandler::~GPUHandler()
	{
		for (int i = 0; i < swapchain.imageViews.size(); i++) vkDestroyImageView(logicalDevice, swapchain.imageViews[i], nullptr);
		vkDestroySwapchainKHR(logicalDevice, swapchain.base, nullptr);
		vkDestroyDevice(logicalDevice, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
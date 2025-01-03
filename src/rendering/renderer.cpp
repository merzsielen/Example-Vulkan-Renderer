/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Renderer.cpp																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
// ...

#include "renderer.h"

namespace VKExample
{
	/*---------------------------------------------------------------------------------------------*/
	/* Renderer																					   */
	/*---------------------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* Command Functions													 */
	/*-----------------------------------------------------------------------*/
	void Renderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer.");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain.extent;

		VkClearValue clearColor = { { { 1.0f, 1.0f, 1.0f, 1.0f } } };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdDraw(commandBuffer, MAX_TRIANGLES * 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer.");
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Render Functions														 */
	/*-----------------------------------------------------------------------*/
	void Renderer::Render()
	{
		vkWaitForFences(device, 1, &inFlights[frame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapChain.base, UINT64_MAX, imagesAvailable[frame], VK_NULL_HANDLE, &imageIndex);

		/*if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized)
		{
			framebufferResized = false;
			RepairSwapchain();
			return;
		}*/

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		vkResetFences(device, 1, &inFlights[frame]);

		vkResetCommandBuffer(commandBuffers[frame], 0);
		RecordCommandBuffer(commandBuffers[frame], imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imagesAvailable[frame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[frame];

		VkSemaphore signalSemaphores[] = { rendersFinished[frame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlights[frame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapchains[] = { swapChain.base };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			// RepairSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	/*-----------------------------------------------------------------------*/
	/* Buffer Functions														 */
	/*-----------------------------------------------------------------------*/
	/* Write Buffer ---------------------------------------------------------*/
	void Renderer::WriteBuffer(Vertex* vertices, unsigned int nVertices)
	{
		unsigned int s = nVertices * sizeof(Vertex);
		void* data;

		vkMapMemory(device, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
		memcpy(data, vertices, s);
		vkUnmapMemory(device, stagingBufferMemory);

		CopyBuffer(stagingBuffer, vertexBuffer, s);
	}

	/*-----------------------------------------------------------------------*/
	/* Vulkan Setup Functions												 */
	/*-----------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* Validation Layers													 */
	/*-----------------------------------------------------------------------*/
	/* Check Validation Layer Support ---------------------------------------*/
	/*
		This function serves to check if the validation layers we are requesting
		are available. This is more important in the debug phase and should be
		disabled (by setting ENABLE_VALIDATION_LAYERS to 0) for release.
	*/
	bool Renderer::CheckValidationLayerSupport(std::vector<const char*> validationLayers)
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

	/*-----------------------------------------------------------------------*/
	/* Instance Setup														 */
	/*-----------------------------------------------------------------------*/
	/* Get Required Extensions ----------------------------------------------*/
	std::vector<const char*> Renderer::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (ENABLE_VALIDATION_LAYERS) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return extensions;
	}

	/* Create Instance ------------------------------------------------------*/
	void Renderer::CreateInstance(	std::string name,
									std::vector<const char*> validationLayers,
									std::vector<const char*> instanceExtensions)
	{
		/*
			Now, we pass some information about our application. This includes
			some basic stuff like its name and engine and whatnot.
		*/
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = name.c_str();
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
		std::vector<const char*> glfwExtensions = GetRequiredExtensions();
		for (int i = 0; i < glfwExtensions.size(); i++) instanceExtensions.push_back(glfwExtensions[i]);
		createInfo.enabledExtensionCount = instanceExtensions.size();
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		if (ENABLE_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		/*
			And now we create our instance.
		*/
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	}

	/*-----------------------------------------------------------------------*/
	/* Surface Setup														 */
	/*-----------------------------------------------------------------------*/
	/* Create Surface -------------------------------------------------------*/
	void Renderer::CreateSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface.");
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Device Setup															 */
	/*-----------------------------------------------------------------------*/
	/* Find Queue Families --------------------------------------------------*/
	QueueFamilyIndices Renderer::FindQueueFamilies(VkPhysicalDevice potentiate)
	{
		QueueFamilyIndices tIndices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(potentiate, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(potentiate, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (int j = 0; j < queueFamilies.size(); j++)
		{
			VkQueueFamilyProperties queueFamily = queueFamilies[j];
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) tIndices.graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(potentiate, i, surface, &presentSupport);

			if (presentSupport) tIndices.presentFamily = i;
			if (tIndices.IsComplete()) break;
			i++;
		}

		return tIndices;
	}

	/* Get SwapChain Support Details ----------------------------------------*/
	SwapChainSupportDetails Renderer::GetSwapChainSupportDetails(VkPhysicalDevice potentiate)
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(potentiate, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(potentiate, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(potentiate, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(potentiate, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(potentiate, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	/* Check Device Extension Support ---------------------------------------*/
	bool Renderer::CheckDeviceExtensionSupport(	VkPhysicalDevice potentiate,
												std::vector<const char*> deviceExtensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(potentiate, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(potentiate, nullptr, &extensionCount, availableExtensions.data());

		for (int i = 0; i < deviceExtensions.size(); i++)
		{
			bool extensionFound = false;

			for (int j = 0; j < availableExtensions.size(); j++)
			{
				if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound) return false;
		}

		return true;
	}

	/* Check Physical Device Suitability ------------------------------------*/
	bool Renderer::CheckPhysicalDeviceSuitability(	VkPhysicalDevice potentiate,
													std::vector<const char*> deviceExtensions)
	{
		QueueFamilyIndices indices = FindQueueFamilies(potentiate);
		if (!CheckDeviceExtensionSupport(potentiate, deviceExtensions)) return false;
		SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(potentiate);
		if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) return false;
		return (indices.IsComplete());
	}

	/* Rate Physical Device -------------------------------------------------*/
	int Renderer::RatePhysicalDevice(VkPhysicalDevice potentiate)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(potentiate, &deviceProperties);
		vkGetPhysicalDeviceFeatures(potentiate, &deviceFeatures);
		if (!deviceFeatures.geometryShader) return 0;
		int score = 0;
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
		score += deviceProperties.limits.maxImageDimension2D;
		return score;
	}

	/* Get Physical Device --------------------------------------------------*/
	void Renderer::GetPhysicalDevice(std::vector<const char*> deviceExtensions)
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
			VkPhysicalDevice potentiate = devices[i];
			if (CheckPhysicalDeviceSuitability(potentiate, deviceExtensions))
			{
				int score = RatePhysicalDevice(potentiate);

				if (score > highestScore)
				{
					bestDevice = potentiate;
					highestScore = score;
				}
			}
		}

		// If the device is still null, that means we found no suitable GPU.
		if (bestDevice == VK_NULL_HANDLE) throw std::runtime_error("Unable to find suitable GPU.");

		physicalDevice = bestDevice;
		indices = FindQueueFamilies(physicalDevice);
	}

	/* Create Device --------------------------------------------------------*/
	void Renderer::CreateDevice(std::vector<const char*> validationLayers,
								std::vector<const char*> deviceExtensions)
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
		createInfo.enabledExtensionCount = deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		createInfo.enabledLayerCount = 0;

		if (ENABLE_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}

		VkDevice logicalDevice = VK_NULL_HANDLE;
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device.");
		}

		device = logicalDevice;
	}

	/* Get Graphics Queue ---------------------------------------------------*/
	void Renderer::GetGraphicsQueue()
	{
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	}

	/* Get Present Queue ----------------------------------------------------*/
	void Renderer::GetPresentQueue()
	{
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

	/*-----------------------------------------------------------------------*/
	/* SwapChain Setup														 */
	/*-----------------------------------------------------------------------*/
	/* Choose Swap Surface Format -------------------------------------------*/
	VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
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

	/* Choose Swap Present Mode ---------------------------------------------*/
	VkPresentModeKHR Renderer::ChooseSwapPresentMode(std::vector<VkPresentModeKHR>& availableModes)
	{
		for (int i = 0; i < availableModes.size(); i++)
		{
			if (availableModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) return availableModes[i];
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Choose Swap Extent ---------------------------------------------------*/
	VkExtent2D Renderer::ChooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { width, height };
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

	/* Create SwapChain -----------------------------------------------------*/
	void Renderer::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = GetSwapChainSupportDetails(physicalDevice);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

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

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchainKHR) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swap chain.");
		}

		std::vector<VkImage> swapchainImages;
		vkGetSwapchainImagesKHR(device, swapchainKHR, &imageCount, nullptr);
		swapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapchainKHR, &imageCount, swapchainImages.data());

		swapChain = { swapchainKHR, swapchainImages, surfaceFormat.format, extent, {} };

		swapChain.imageViews.resize(swapchainImages.size());

		for (int i = 0; i < swapchainImages.size(); i++)
		{
			VkImageViewCreateInfo imgCreateInfo{};
			imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgCreateInfo.image = swapchainImages[i];
			imgCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imgCreateInfo.format = swapChain.format;
			imgCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgCreateInfo.subresourceRange.baseMipLevel = 0;
			imgCreateInfo.subresourceRange.levelCount = 1;
			imgCreateInfo.subresourceRange.baseArrayLayer = 0;
			imgCreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &imgCreateInfo, nullptr, &swapChain.imageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image views.");
			}
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Framebuffer Setup													 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupFramebuffers()
	{
		framebuffers.resize(swapChain.imageViews.size());

		for (int i = 0; i < swapChain.imageViews.size(); i++)
		{
			VkImageView attachments[] = { swapChain.imageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChain.extent.width;
			framebufferInfo.height = swapChain.extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer.");
			}
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Render Pass Setup													 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupRenderPasses()
	{
		/* Render Pass Setup ------------------------------------------------*/
		/*
			Next, we need to create our render pass.
		*/
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = swapChain.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; // This index corresponds to the fragment shader input.
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass.");
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Pipeline Setup														 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupPipeline(std::vector<VkDynamicState> dynamicStates, Shader baseShader)
	{
		/* Pipeline Setup ---------------------------------------------------*/
		/*
			We have to specify certain states we would like not
			to be baked into the pipeline.
		*/
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();

		/*
			Now we specify our vertex attribute information (the data
			we'll be passing to our vertex shader).
		*/
		VkVertexInputBindingDescription vertexBindingDescription = Vertex::BindingDescription();
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions = Vertex::AttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

		/*
			Next, we specify our input assembly.
		*/
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		/*
			Next, we set up our viewport. I may move this to the
			camera class shortly.

			Here, we also set up our scissor.
		*/
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapChain.extent.width;
		viewport.height = (float)swapChain.extent.height;

		scissor.offset = { 0, 0 };
		scissor.extent = swapChain.extent;

		/*
			Now, we prepare our rasterizer.
		*/
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		/*rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;*/

		/*
			There is an endless parade of states we must create.
			Next is multisampling.
		*/
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		/*multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;*/

		/*
			And now the color blend attachment state.
		*/
		// Consider setting blendEnable to VK_TRUE, srcColorBlendFactor to VK_BLEND_FACTOR_SRC_ALPHA,
		// and dstColorBlendFactor to VK_BLEND_FACTOR_ZEROVK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA.
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		// Consider setting logicOpEnable to VK_FALSEVK_TRUE and logicOp to VK_LOGIC_OP_COPYVK_LOGIC_OP_AND.
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		/*
			Next, we specify our pipeline layout.
		*/
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout.");
		}

		/* Pipeline Finalization --------------------------------------------*/
		/*
			Now, at long last, we can finalize our pipeline. God is good.
		*/
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages = baseShader.GetStages();
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create graphics pipeline.");
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Buffer Setup															 */
	/*-----------------------------------------------------------------------*/
	/* Find Memory Type -----------------------------------------------------*/
	unsigned int Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (int i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	/* Copy Buffer ----------------------------------------------------------*/
	void Renderer::CopyBuffer(VkBuffer src, VkBuffer dst, unsigned int size)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	/* Create Buffer --------------------------------------------------------*/
	void Renderer::CreateBuffer(unsigned int size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer.");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory.");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	/* Setup Vertex Buffer --------------------------------------------------*/
	void Renderer::SetupVertexBuffer()
	{
		unsigned int bufferSize = sizeof(Vertex) * (MAX_TRIANGLES * 3);
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
	}

	/*-----------------------------------------------------------------------*/
	/* Command Setup														 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupCommands()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool.");
		}

		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers.");
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Synchronization Setup												 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupSynchronization()
	{
		imagesAvailable.resize(MAX_FRAMES_IN_FLIGHT);
		rendersFinished.resize(MAX_FRAMES_IN_FLIGHT);
		inFlights.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imagesAvailable[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &rendersFinished[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlights[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create semaphores and fences.");
			}
		}
	}

	/*-----------------------------------------------------------------------*/
	/* Constructor															 */
	/*-----------------------------------------------------------------------*/
	Renderer::Renderer(std::vector<VkDynamicState> dynamicStates, int screenWidth, int screenHeight, const char* title)
	{
		/*-----------------------------------------------*/
		/* Preliminaries								 */
		/*-----------------------------------------------*/
		this->frame = 0;
		this->windowResized = false;

		/*-----------------------------------------------*/
		/* GLFW Setup									 */
		/*-----------------------------------------------*/
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(screenWidth, screenHeight, title, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, ResizeCallback);

		/*-----------------------------------------------*/
		/* Vulkan Setup									 */
		/*-----------------------------------------------*/
		/* Validation Layers ----------------------------*/
		std::vector<const char*> validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};

		if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport(validationLayers))
		{
			throw std::runtime_error("Some of the requested validation layers are not available.");
		}

		/* Instance -------------------------------------*/
		std::vector<const char*> instanceExtensions
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
		};

		CreateInstance(title, validationLayers, instanceExtensions);

		/* Surface --------------------------------------*/
		CreateSurface();

		/* Devices --------------------------------------*/
		std::vector<const char*> deviceExtensions
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME
		};

		GetPhysicalDevice(deviceExtensions);
		CreateDevice(validationLayers, deviceExtensions);
		GetGraphicsQueue();
		GetPresentQueue();

		/* SwapChain ------------------------------------*/
		CreateSwapChain();

		/* Shaders --------------------------------------*/
		Shader baseShader = Shader(device, "assets/shaders/base_vert.spv", "assets/shaders/base_frag.spv");
		shaders["base"] = baseShader;

		/* Render Pass Setup ----------------------------*/
		SetupRenderPasses();

		/* Pipeline Setup -------------------------------*/
		SetupPipeline(dynamicStates, baseShader);

		/* Render Pass Setup ----------------------------*/
		SetupFramebuffers();

		/* Command Pool & Buffer Setup ------------------*/
		SetupCommands();

		/* Vertex Buffer Setup --------------------------*/
		SetupVertexBuffer();

		/* Synchronization Setup ------------------------*/
		SetupSynchronization();

		/*    TEMPORARY TEMPORARY TEMPORARY TEMPORARY    */
		memset(vertices, 0, MAX_TRIANGLES * 3);

		unsigned int size = floor(sqrt((MAX_TRIANGLES / 2)));
		float d = 1.0 / size;

		glm::vec2 offset = { -1.0f + d, 1.0f + d };

		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				int i = x + (y * size);
				int o = i * 6;

				float fx = x / (size / 2.0);
				float fy = y / (size / 2.0);

				vertices[o] = { { -d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 1.0f, 0.0f, 0.0f, 1.0f } };
				vertices[o + 1] = { { -d + fx + offset.x, -d - fy + offset.y, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f} };
				vertices[o + 2] = { { d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f } };
				vertices[o + 3] = { { -d + fx + offset.x, -d - fy + offset.y, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f} };
				vertices[o + 4] = { { d + fx + offset.x, -d - fy + offset.y, 0.0f },	{ 1.0f, 1.0f, 1.0f, 1.0f } };
				vertices[o + 5] = { { d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f } };
			}
		}

		WriteBuffer(vertices, MAX_TRIANGLES * 3);
		/*    TEMPORARY TEMPORARY TEMPORARY TEMPORARY    */
	}

	/*-----------------------------------------------------------------------*/
	/* Deconstructor														 */
	/*-----------------------------------------------------------------------*/
	Renderer::~Renderer()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device, imagesAvailable[i], nullptr);
			vkDestroySemaphore(device, rendersFinished[i], nullptr);
			vkDestroyFence(device, inFlights[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		for (int i = 0; i < framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);

		for (int i = 0; i < swapChain.imageViews.size(); i++) vkDestroyImageView(device, swapChain.imageViews[i], nullptr);
		vkDestroySwapchainKHR(device, swapChain.base, nullptr);

		/*vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);*/

		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
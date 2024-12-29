/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Renderer.h																											 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#include "renderer.h"

namespace Untitled
{
	/*---------------------------------------------------------------------------------------------*/
	/* Renderer																					   */
	/*---------------------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* Setup Functions														 */
	/*-----------------------------------------------------------------------*/
	void Renderer::SetupFramebuffers()
	{
		/* Frame Buffer Setup -----------------------------------------------*/
		/*
			Because God wishes to spite me, he has forced us to create
			frame buffers. Because evidently all our work so far is not
			enough. Vulkan hungers.
		*/
		framebuffers.resize(swapchain.imageViews.size());

		for (int i = 0; i < swapchain.imageViews.size(); i++)
		{
			VkImageView attachments[] = { swapchain.imageViews[i] };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer.");
			}
		}
	}

	void Renderer::SetupRenderPasses()
	{
		/* Render Pass Setup ------------------------------------------------*/
		/*
			Next, we need to create our render pass.
		*/
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = gpuHandler->Swapchain().format;
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
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;

		scissor.offset = { 0, 0 };
		scissor.extent = extent;

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

	unsigned int Renderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpuHandler->PhysicalDevice(), &memProperties);

		for (int i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

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

	void Renderer::SetupVertexBuffer()
	{
		unsigned int vertexBufferSize = sizeof(Vertex) * (MAX_TRIANGLES * 3);
		CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

		void* data;
		vkMapMemory(device, vertexBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);
		memcpy(data, vertices, vertexBufferSize);
		vkUnmapMemory(device, vertexBufferMemory);
	}

	void Renderer::SetupCommands()
	{
		/* Command Pool & Buffer Setup --------------------------------------*/
		/*
			I feel like I'm scraping my bare knuckles on the GPU right
			now. Why in God's good name is this so tedious?
		*/
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(gpuHandler->PhysicalDevice(), gpuHandler->Surface());

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

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

	void Renderer::SetupSynchronization()
	{
		/* Synchronization Setup --------------------------------------------*/
		/*
			So that the CPU and GPU don't fall out of line, we have to
			sync them up.
		*/
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
	/* Render Functions														 */
	/*-----------------------------------------------------------------------*/
	void Renderer::Render()
	{
		vkWaitForFences(device, 1, &inFlights[frame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(device, swapchain.base, UINT64_MAX, imagesAvailable[frame], VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || gpuHandler->CheckFramebufferSize())
		{
			gpuHandler->ResizeFramebuffer(false);
			RepairSwapchain();
			return;
		}
		
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

		VkSwapchainKHR swapchains[] = { swapchain.base };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		{
			RepairSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to acquire swap chain image.");
		}

		frame = (frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	/*-----------------------------------------------------------------------*/
	/* Command Functions													 */
	/*-----------------------------------------------------------------------*/
	/* Record Command Buffer ------------------------------------------------*/
	/*
		In order to pass a command, we need to write it to the buffer.
	*/
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
		renderPassInfo.renderArea.extent = extent;

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

	/*-------------------------------------------------------------------*/
	/* Repair Swapchain													 */
	/*-------------------------------------------------------------------*/
	void Renderer::RepairSwapchain()
	{
		int width = 0, height = 0;
		GLFWwindow* window = gpuHandler->Window();
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);

		for (int i = 0; i < framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		}

		for (size_t i = 0; i < swapchain.imageViews.size(); i++)
		{
			vkDestroyImageView(device, swapchain.imageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, swapchain.base, nullptr);

		gpuHandler->RepairSwapchain();
		swapchain = gpuHandler->Swapchain();
		SetupFramebuffers();
	}

	/*-----------------------------------------------------------------------*/
	/* Constructor															 */
	/*-----------------------------------------------------------------------*/
	Renderer::Renderer(std::vector<VkDynamicState> dynamicStates, GPUHandler* gpuHandler)
	{
		// TEST
		memset(vertices, 0, MAX_TRIANGLES * 3);

		unsigned int size = floor(sqrt((MAX_TRIANGLES / 2)));
		float d = 1.0 / size;

		Vec2 offset = { -1.0f + d, 1.0f + d };

		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				int i = x + (y * size);
				int o = i * 6;

				float fx = x / (size / 2.0);
				float fy = y / (size / 2.0);

				vertices[o] =		{ { -d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 1.0f, 0.0f, 0.0f, 1.0f } };
				vertices[o + 1] =	{ { -d + fx + offset.x, -d - fy + offset.y, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f}};
				vertices[o + 2] =	{ { d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f } };
				vertices[o + 3] =	{ { -d + fx + offset.x, -d - fy + offset.y, 0.0f},	{0.0f, 1.0f, 0.0f, 1.0f}};
				vertices[o + 4] =	{ { d + fx + offset.x, -d - fy + offset.y, 0.0f },	{ 1.0f, 1.0f, 1.0f, 1.0f } };
				vertices[o + 5] =	{ { d + fx + offset.x, d - fy + offset.y, 0.0f },	{ 0.0f, 0.0f, 1.0f, 1.0f } };
			}
		}

		/*
			First, a few quick assignments.
		*/
		this->frame = 0;

		this->gpuHandler = gpuHandler;

		VkPhysicalDevice physicalDevice = gpuHandler->PhysicalDevice();
		device = gpuHandler->LogicalDevice();
		VkSurfaceKHR surface = gpuHandler->Surface();

		swapchain = gpuHandler->Swapchain();
		extent = swapchain.extent;

		this->graphicsQueue = gpuHandler->GraphicsQueue();
		this->presentQueue = gpuHandler->PresentQueue();

		/* Shader Setup -----------------------------------------------------*/
		/*
			We need to set up our basic shader.
		*/
		Shader baseShader = Shader(gpuHandler->LogicalDevice(), "assets/shaders/base_vert.spv", "assets/shaders/base_frag.spv");
		shaders["base"] = baseShader;

		/* Render Pass Setup ------------------------------------------------*/
		SetupRenderPasses();

		/* Pipeline Setup ---------------------------------------------------*/
		SetupPipeline(dynamicStates, baseShader);

		/* Render Pass Setup ------------------------------------------------*/
		SetupFramebuffers();

		/* Vertex Buffer Setup ----------------------------------------------*/
		SetupVertexBuffer();

		/* Command Pool & Buffer Setup --------------------------------------*/
		SetupCommands();

		/* Synchronization Setup --------------------------------------------*/
		SetupSynchronization();
	}

	/*-------------------------------------------------------------------*/
	/* Deconstructor													 */
	/*-------------------------------------------------------------------*/
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

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);
	}
}
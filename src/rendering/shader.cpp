/*-----------------------------------------------------------------------------------------------------------------------*/
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/* Shader.h																												 */
/* --  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- */
/*-----------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------*/
/* Includes																						   */
/*-------------------------------------------------------------------------------------------------*/
#include "shader.h"

namespace VkExample
{
	/*---------------------------------------------------------------------------------------------*/
	/* Utility Functions																		   */
	/*---------------------------------------------------------------------------------------------*/
	/* ReadCode -------------------------------------------------------------*/
	/*
		ReadCode() takes in the path to a file containing shader code
		and returns the code, if read properly.

		Input: Shader path
		Output: Shader code
	*/
	std::vector<char> ReadCode(const char* path)
	{
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open shader file: " + std::string(path) + ".");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	/* Create Shader Module -------------------------------------------------*/
	/*
		CreateShaderModule() tells Vulkan to compile a shader for the
		given code.
	*/
	VkShaderModule CreateShaderModule(VkDevice device, std::vector<char> code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module.");
		}

		return shaderModule;
	}

	/*---------------------------------------------------------------------------------------------*/
	/* Shader																					   */
	/*---------------------------------------------------------------------------------------------*/
	/*-----------------------------------------------------------------------*/
	/* Destroy Modules														 */
	/*-----------------------------------------------------------------------*/
	void Shader::DestroyModules(VkDevice device)
	{
		for (int i = 0; i < modules.size(); i++)
		{
			vkDestroyShaderModule(device, modules[i], nullptr);
		}

		modules.clear();
	}

	/*-----------------------------------------------------------------------*/
	/* Constructors															 */
	/*-----------------------------------------------------------------------*/
	Shader::Shader() {};

	Shader::Shader(VkDevice device, std::string vertexPath, std::string fragmentPath)
	{
		VkShaderModule vertexModule = CreateShaderModule(device, ReadCode(vertexPath.c_str()));
		VkShaderModule fragmentModule = CreateShaderModule(device, ReadCode(fragmentPath.c_str()));

		VkPipelineShaderStageCreateInfo vertexStageInfo{};
		vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageInfo.module = vertexModule;
		vertexStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentStageInfo{};
		fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentStageInfo.module = fragmentModule;
		fragmentStageInfo.pName = "main";

		stages.push_back(vertexStageInfo);
		stages.push_back(fragmentStageInfo);
		modules.push_back(vertexModule);
		modules.push_back(fragmentModule);
	}
}
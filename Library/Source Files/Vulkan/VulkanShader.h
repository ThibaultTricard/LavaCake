#ifndef VULKAN_SHADER
#define VULKAN_SHADER

#include "Common.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptor.h"
#include "VulkanViewPort.h"
#include "Tools.h"

namespace LavaCake {
	namespace Shader {
		struct ShaderStageParameters {
			VkShaderStageFlagBits        ShaderStage;
			VkShaderModule               ShaderModule;
			char const                 * EntryPointName;
			VkSpecializationInfo const * SpecializationInfo;
		};

		bool CreateShaderModule(VkDevice                           logical_device,
			std::vector<unsigned char> const & source_code,
			VkShaderModule                   & shader_module);

		void DestroyShaderModule(VkDevice         logical_device,
			VkShaderModule & shader_module);
	}
}

#endif
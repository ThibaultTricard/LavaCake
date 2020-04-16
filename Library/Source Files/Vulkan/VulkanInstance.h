#ifndef VULKAN_INSTANCE
#define VULKAN_INSTANCE

#include "Common.h"
#include "VulkanQueue.h"

namespace LavaCake {
	namespace Instance {


		const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
		};


		bool CreateVulkanInstance(std::vector<char const *> const & desired_extensions,
			char const * const                application_name,
			VkInstance                      & instance);

		void DestroyVulkanInstance(VkInstance & instance);

		bool CreateVulkanInstanceWithWsiExtensionsEnabled(std::vector<char const *> & desired_extensions,
			char const * const          application_name,
			VkInstance                & instance);

		bool CreateLogicalDeviceWithWsiExtensionsEnabled(VkPhysicalDevice            physical_device,
			std::vector< Queue::QueueInfo >    queue_infos,
			std::vector<char const *> & desired_extensions,
			VkPhysicalDeviceFeatures  * desired_features,
			VkDevice                  & logical_device);

	}
}

#endif
#ifndef VULKAN_QUEUE
#define VULKAN_QUEUE

#include "Common.h"

namespace LavaCake {
	namespace Queue {
		struct QueueInfo {
			uint32_t           FamilyIndex;
			std::vector<float> Priorities;
		};

		void GetDeviceQueue(VkDevice logical_device, uint32_t queue_family_index, uint32_t queue_index, VkQueue & queue);
		bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice                       physical_device,
			std::vector<VkQueueFamilyProperties> & queue_families);
		bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice   physical_device,
			VkQueueFlags       desired_capabilities,
			uint32_t         & queue_family_index);
	}
}

#endif
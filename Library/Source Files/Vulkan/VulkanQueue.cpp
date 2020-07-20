#include "VulkanQueue.h"

namespace LavaCake {
	namespace Queue {
		bool CheckAvailableQueueFamiliesAndTheirProperties(VkPhysicalDevice                       physical_device,
			std::vector<VkQueueFamilyProperties> & queue_families) {
			uint32_t queue_families_count = 0;

			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, nullptr);
			if (queue_families_count == 0) {
				std::cout << "Could not get the number of queue families." << std::endl;
				return false;
			}

			queue_families.resize(queue_families_count);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_families_count, queue_families.data());
			if (queue_families_count == 0) {
				std::cout << "Could not acquire properties of queue families." << std::endl;
				return false;
			}

			return true;
		}

		bool SelectIndexOfQueueFamilyWithDesiredCapabilities(VkPhysicalDevice   physical_device,
			VkQueueFlags       desired_capabilities,
			uint32_t         & queue_family_index) {
			std::vector<VkQueueFamilyProperties> queue_families;
			if (!CheckAvailableQueueFamiliesAndTheirProperties(physical_device, queue_families)) {
				return false;
			}

			for (uint32_t index = 0; index < static_cast<uint32_t>(queue_families.size()); ++index) {
				if ((queue_families[index].queueCount > 0) &&
					((queue_families[index].queueFlags & desired_capabilities) == desired_capabilities)) {
					queue_family_index = index;
					return true;
				}
			}
			return false;
		}

	}
}
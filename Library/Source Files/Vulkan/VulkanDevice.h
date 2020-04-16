#ifndef VULKAN_DEVICE
#define VULKAN_DEVICE

#include "Common.h"
#include "VulkanQueue.h"

namespace LavaCake {
	namespace Device {
		namespace Physical {
			bool EnumerateAvailablePhysicalDevices(VkInstance                      instance,
				std::vector<VkPhysicalDevice> & available_devices);

			bool CheckAvailableDeviceExtensions(VkPhysicalDevice                     physical_device,
				std::vector<VkExtensionProperties> & available_extensions);

			void GetFeaturesAndPropertiesOfPhysicalDevice(VkPhysicalDevice             physical_device,
				VkPhysicalDeviceFeatures   & device_features,
				VkPhysicalDeviceProperties & device_properties);
		}
		namespace Logical {
			bool CreateLogicalDevice(VkPhysicalDevice                  physical_device,
				std::vector< Queue::QueueInfo >          queue_infos,
				std::vector<char const *> const & desired_extensions,
				VkPhysicalDeviceFeatures        * desired_features,
				VkDevice                        & logical_device);

			bool CreateLogicalDeviceWithGeometryShadersAndGraphicsAndComputeQueues(VkInstance   instance,
				VkDevice   & logical_device,
				VkQueue    & graphics_queue,
				VkQueue    & compute_queue);

			void DestroyLogicalDevice(VkDevice & logical_device);
		}
	}
}

#endif
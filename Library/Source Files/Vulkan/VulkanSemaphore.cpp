#include "VulkanSemaphore.h"

namespace LavaCake {
	namespace Semaphore {
		bool CreateSemaphore(VkDevice      logical_device,
			VkSemaphore & semaphore) {
			VkSemaphoreCreateInfo semaphore_create_info = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,    // VkStructureType            sType
				nullptr,                                    // const void               * pNext
				0                                           // VkSemaphoreCreateFlags     flags
			};

			VkResult result = vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &semaphore);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a semaphore." << std::endl;
				return false;
			}
			return true;
		}

		void DestroySemaphore(VkDevice      logical_device,
			VkSemaphore & semaphore) {
			if (VK_NULL_HANDLE != semaphore) {
				vkDestroySemaphore(logical_device, semaphore, nullptr);
				semaphore = VK_NULL_HANDLE;
			}
		}
	}
}
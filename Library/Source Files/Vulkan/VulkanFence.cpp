#include "VulkanFence.h" 

namespace LavaCake {

	namespace Fence {
		bool CreateFence(VkDevice   logical_device,
			bool       signaled,
			VkFence  & fence) {
			VkFenceCreateInfo fence_create_info = {
				VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,          // VkStructureType        sType
				nullptr,                                      // const void           * pNext
				signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u, // VkFenceCreateFlags     flags
			};

			VkResult result = vkCreateFence(logical_device, &fence_create_info, nullptr, &fence);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a fence." << std::endl;
				return false;
			}
			return true;
		}

		bool WaitForFences(VkDevice                     logical_device,
			std::vector<VkFence> const & fences,
			VkBool32                     wait_for_all,
			uint64_t                     timeout) {
			if (fences.size() > 0) {
				VkResult result = vkWaitForFences(logical_device, static_cast<uint32_t>(fences.size()), fences.data(), wait_for_all, timeout);
				if (VK_SUCCESS != result) {
					std::cout << "Waiting on fence failed." << std::endl;
					return false;
				}
				return true;
			}
			return false;
		}

		bool ResetFences(VkDevice                     logical_device,
			std::vector<VkFence> const & fences) {
			if (fences.size() > 0) {
				VkResult result = vkResetFences(logical_device, static_cast<uint32_t>(fences.size()), fences.data());
				if (VK_SUCCESS != result) {
					std::cout << "Error occurred when tried to reset fences." << std::endl;
					return false;
				}
				return VK_SUCCESS == result;
			}
			return false;
		}

		void DestroyFence(VkDevice   logical_device,
			VkFence  & fence) {
			if (VK_NULL_HANDLE != fence) {
				vkDestroyFence(logical_device, fence, nullptr);
				fence = VK_NULL_HANDLE;
			}
		}
	}

}
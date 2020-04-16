#ifndef VULKAN_FENCE
#define VULKAN_FENCE

#include "Common.h"

namespace LavaCake {
	namespace Fence {
		bool CreateFence(VkDevice   logical_device,
			bool       signaled,
			VkFence  & fence);

		bool WaitForFences(VkDevice                     logical_device,
			std::vector<VkFence> const & fences,
			VkBool32                     wait_for_all,
			uint64_t                     timeout);

		bool ResetFences(VkDevice                     logical_device,
			std::vector<VkFence> const & fences);

		void DestroyFence(VkDevice   logical_device,
			VkFence  & fence);

	}
}


#endif // !VULKAN_FENCE

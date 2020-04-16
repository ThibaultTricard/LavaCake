#ifndef VULKAN_SEMAPHORE
#define VULKAN_SEMAPHORE

#include "Common.h"

namespace LavaCake {
	namespace Semaphore {
		struct WaitSemaphoreInfo {
			VkSemaphore           Semaphore;
			VkPipelineStageFlags  WaitingStage;
		};
		bool CreateSemaphore(VkDevice      logical_device,
			VkSemaphore & semaphore);

		void DestroySemaphore(VkDevice      logical_device,
			VkSemaphore & semaphore);
	}
}

#endif // !1
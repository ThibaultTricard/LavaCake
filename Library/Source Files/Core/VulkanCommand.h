// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and / or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The below copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// � Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Chapter: 03 Command Buffers and Synchronization
// Recipe:  01 Creating a command pool

#ifndef VULKAN_COMMAND
#define VULKAN_COMMAND

#include "Common.h"
#include "VulkanSemaphore.h"

namespace LavaCake {
	
	namespace Command {
		bool CreateCommandPool(VkDevice                   logical_device,
			VkCommandPoolCreateFlags   command_pool_flags,
			uint32_t                   queue_family,
			VkCommandPool            & command_pool);

		bool AllocateCommandBuffers(VkDevice                       logical_device,
			VkCommandPool                  command_pool,
			VkCommandBufferLevel           level,
			uint32_t                       count,
			std::vector<VkCommandBuffer> & command_buffers);

		bool BeginCommandBufferRecordingOperation(VkCommandBuffer                  command_buffer,
			VkCommandBufferUsageFlags        usage,
			VkCommandBufferInheritanceInfo * secondary_command_buffer_info);

		bool ResetCommandBuffer(VkCommandBuffer command_buffer,
			bool            release_resources);

		bool EndCommandBufferRecordingOperation(VkCommandBuffer command_buffer);

		bool ResetCommandPool(VkDevice      logical_device,
			VkCommandPool command_pool,
			bool          release_resources);

		bool SubmitCommandBuffersToQueue(VkQueue                         queue,
			std::vector<Semaphore::WaitSemaphoreInfo>  wait_semaphore_infos,
			std::vector<VkCommandBuffer>    command_buffers,
			std::vector<VkSemaphore>        signal_semaphores,
			VkFence                         fence);

		bool SynchronizeTwoCommandBuffers(VkQueue                         first_queue,
			std::vector<Semaphore::WaitSemaphoreInfo>  first_wait_semaphore_infos,
			std::vector<VkCommandBuffer>    first_command_buffers,
			std::vector<Semaphore::WaitSemaphoreInfo>  synchronizing_semaphores,
			VkQueue                         second_queue,
			std::vector<VkCommandBuffer>    second_command_buffers,
			std::vector<VkSemaphore>        second_signal_semaphores,
			VkFence                         second_fence);

		bool CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice                         logical_device,
			VkQueue                          queue,
			std::vector<Semaphore::WaitSemaphoreInfo>   wait_semaphore_infos,
			std::vector<VkCommandBuffer>     command_buffers,
			std::vector<VkSemaphore>         signal_semaphores,
			VkFence                          fence,
			uint64_t                         timeout,
			VkResult                       & wait_status);

		void ExecuteSecondaryCommandBufferInsidePrimaryCommandBuffer(VkCommandBuffer                      primary_command_buffer,
			std::vector<VkCommandBuffer> const & secondary_command_buffers);

		bool WaitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue queue);

		bool WaitForAllSubmittedCommandsToBeFinished(VkDevice logical_device);

		void FreeCommandBuffers(VkDevice                       logical_device,
			VkCommandPool                  command_pool,
			std::vector<VkCommandBuffer> & command_buffers);

		void DestroyCommandPool(VkDevice        logical_device,
			VkCommandPool & command_pool);
	}
	
} // namespace LavaCake

#endif // CREATING_A_COMMAND_POOL
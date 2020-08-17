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
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329

#include "VulkanCommand.h"
#include "VulkanFence.h"

namespace LavaCake {
	
	namespace Command {
		bool CreateCommandPool(VkDevice                   logical_device,
			VkCommandPoolCreateFlags   parameters,
			uint32_t                   queue_family,
			VkCommandPool            & command_pool) {
			VkCommandPoolCreateInfo command_pool_create_info = {
				VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,   // VkStructureType              sType
				nullptr,                                      // const void                 * pNext
				parameters,                                   // VkCommandPoolCreateFlags     flags
				queue_family                                  // uint32_t                     queueFamilyIndex
			};

			VkResult result = vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create command pool." << std::endl;
				return false;
			}
			return true;
		}

		bool AllocateCommandBuffers(VkDevice                       logical_device,
			VkCommandPool                  command_pool,
			VkCommandBufferLevel           level,
			uint32_t                       count,
			std::vector<VkCommandBuffer> & command_buffers) {
			VkCommandBufferAllocateInfo command_buffer_allocate_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // VkStructureType          sType
				nullptr,                                          // const void             * pNext
				command_pool,                                     // VkCommandPool            commandPool
				level,                                            // VkCommandBufferLevel     level
				count                                             // uint32_t                 commandBufferCount
			};

			command_buffers.resize(count);

			VkResult result = vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, command_buffers.data());
			if (VK_SUCCESS != result) {
				std::cout << "Could not allocate command buffers." << std::endl;
				return false;
			}
			return true;
		}

		bool BeginCommandBufferRecordingOperation(VkCommandBuffer                  command_buffer,
			VkCommandBufferUsageFlags        usage,
			VkCommandBufferInheritanceInfo * secondary_command_buffer_info) {
			VkCommandBufferBeginInfo command_buffer_begin_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,    // VkStructureType                        sType
				nullptr,                                        // const void                           * pNext
				usage,                                          // VkCommandBufferUsageFlags              flags
				secondary_command_buffer_info                   // const VkCommandBufferInheritanceInfo * pInheritanceInfo
			};

			VkResult result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
			if (VK_SUCCESS != result) {
				std::cout << "Could not begin command buffer recording operation." << std::endl;
				return false;
			}
			return true;
		}

		bool EndCommandBufferRecordingOperation(VkCommandBuffer command_buffer) {
			VkResult result = vkEndCommandBuffer(command_buffer);
			if (VK_SUCCESS != result) {
				std::cout << "Error occurred during command buffer recording." << std::endl;
				return false;
			}
			return true;
		}
	
		bool ResetCommandBuffer(VkCommandBuffer command_buffer,
			bool            release_resources) {
			VkResult result = vkResetCommandBuffer(command_buffer, release_resources ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
			if (VK_SUCCESS != result) {
				std::cout << "Error occurred during command buffer reset." << std::endl;
				return false;
			}
			return true;
		}

		bool ResetCommandPool(VkDevice      logical_device,
			VkCommandPool command_pool,
			bool          release_resources) {
			VkResult result = vkResetCommandPool(logical_device, command_pool, release_resources ? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT : 0);
			if (VK_SUCCESS != result) {
				std::cout << "Error occurred during command pool reset." << std::endl;
				return false;
			}
			return true;
		}

		bool SubmitCommandBuffersToQueue(VkQueue                         queue,
			std::vector<Semaphore::WaitSemaphoreInfo>  wait_semaphore_infos,
			std::vector<VkCommandBuffer>    command_buffers,
			std::vector<VkSemaphore>        signal_semaphores,
			VkFence                         fence) {
			std::vector<VkSemaphore>          wait_semaphore_handles;
			std::vector<VkPipelineStageFlags> wait_semaphore_stages;

			for (auto & wait_semaphore_info : wait_semaphore_infos) {
				wait_semaphore_handles.emplace_back(wait_semaphore_info.Semaphore);
				wait_semaphore_stages.emplace_back(wait_semaphore_info.WaitingStage);
			}

			VkSubmitInfo submit_info = {
				VK_STRUCTURE_TYPE_SUBMIT_INFO,                        // VkStructureType                sType
				nullptr,                                              // const void                   * pNext
				static_cast<uint32_t>(wait_semaphore_infos.size()),   // uint32_t                       waitSemaphoreCount
				wait_semaphore_handles.data(),                        // const VkSemaphore            * pWaitSemaphores
				wait_semaphore_stages.data(),                         // const VkPipelineStageFlags   * pWaitDstStageMask
				static_cast<uint32_t>(command_buffers.size()),        // uint32_t                       commandBufferCount
				command_buffers.data(),                               // const VkCommandBuffer        * pCommandBuffers
				static_cast<uint32_t>(signal_semaphores.size()),      // uint32_t                       signalSemaphoreCount
				signal_semaphores.data()                              // const VkSemaphore            * pSignalSemaphores
			};

			VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
			if (VK_SUCCESS != result) {
				std::cout << "Error occurred during command buffer submission." << std::endl;
				return false;
			}
			return true;
		}

		bool SynchronizeTwoCommandBuffers(VkQueue                         first_queue,
			std::vector<Semaphore::WaitSemaphoreInfo>  first_wait_semaphore_infos,
			std::vector<VkCommandBuffer>    first_command_buffers,
			std::vector<Semaphore::WaitSemaphoreInfo>  synchronizing_semaphores,
			VkQueue                         second_queue,
			std::vector<VkCommandBuffer>    second_command_buffers,
			std::vector<VkSemaphore>        second_signal_semaphores,
			VkFence                         second_fence) {
			std::vector<VkSemaphore> first_signal_semaphores;
			for (auto & semaphore_info : synchronizing_semaphores) {
				first_signal_semaphores.emplace_back(semaphore_info.Semaphore);
			}
			if (!SubmitCommandBuffersToQueue(first_queue, first_wait_semaphore_infos, first_command_buffers, first_signal_semaphores, VK_NULL_HANDLE)) {
				return false;
			}

			if (!SubmitCommandBuffersToQueue(second_queue, synchronizing_semaphores, second_command_buffers, second_signal_semaphores, second_fence)) {
				return false;
			}
			return true;
		}

		bool CheckIfProcessingOfSubmittedCommandBufferHasFinished(VkDevice                         logical_device,
			VkQueue                          queue,
			std::vector<Semaphore::WaitSemaphoreInfo>   wait_semaphore_infos,
			std::vector<VkCommandBuffer>     command_buffers,
			std::vector<VkSemaphore>         signal_semaphores,
			VkFence                          fence,
			uint64_t                         timeout,
			VkResult                       & wait_status) {
			if (!SubmitCommandBuffersToQueue(queue, wait_semaphore_infos, command_buffers, signal_semaphores, fence)) {
				return false;
			}

			return Fence::WaitForFences(logical_device, { fence }, VK_FALSE, timeout);
		}


		void ExecuteSecondaryCommandBufferInsidePrimaryCommandBuffer(VkCommandBuffer                      command_buffer,
			std::vector<VkCommandBuffer> const & secondary_command_buffers) {
			if (secondary_command_buffers.size() > 0) {
				vkCmdExecuteCommands(command_buffer, static_cast<uint32_t>(secondary_command_buffers.size()), secondary_command_buffers.data());
			}
		}

		bool WaitUntilAllCommandsSubmittedToQueueAreFinished(VkQueue queue) {
			VkResult result = vkQueueWaitIdle(queue);
			if (VK_SUCCESS != result) {
				std::cout << "Waiting for all operations submitted to queue failed." << std::endl;
				return false;
			}
			return true;
		}

		bool WaitForAllSubmittedCommandsToBeFinished(VkDevice logical_device) {
			VkResult result = vkDeviceWaitIdle(logical_device);
			if (VK_SUCCESS != result) {
				std::cout << "Waiting on a device failed." << std::endl;
				return false;
			}
			return true;
		}

		void FreeCommandBuffers(VkDevice                       logical_device,
			VkCommandPool                  command_pool,
			std::vector<VkCommandBuffer> & command_buffers) {
			if (command_buffers.size() > 0) {
				vkFreeCommandBuffers(logical_device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
				command_buffers.clear();
			}
		}

		void DestroyCommandPool(VkDevice        logical_device,
			VkCommandPool & command_pool) {
			if (VK_NULL_HANDLE != command_pool) {
				vkDestroyCommandPool(logical_device, command_pool, nullptr);
				command_pool = VK_NULL_HANDLE;
			}
		}
	}

} // namespace LavaCake

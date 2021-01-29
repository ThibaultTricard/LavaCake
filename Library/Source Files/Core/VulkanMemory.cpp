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


#include "VulkanLoader.h"
#include "VulkanFence.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace LavaCake {
	namespace Core {
		bool MapUpdateAndUnmapHostVisibleMemory(VkDevice             logical_device,
			VkDeviceMemory       memory_object,
			VkDeviceSize         offset,
			VkDeviceSize         data_size,
			void               * data,
			bool                 unmap,
			void             * * pointer) {
			VkResult result;
			void * local_pointer;
			result = vkMapMemory(logical_device, memory_object, offset, data_size, 0, &local_pointer);
			if (VK_SUCCESS != result) {
				std::cout << "Could not map memory object." << std::endl;
				return false;
			}

			std::memcpy(local_pointer, data, static_cast<size_t>(data_size));

			std::vector<VkMappedMemoryRange> memory_ranges = {
				{
					VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType
					nullptr,                                // const void       * pNext
					memory_object,                          // VkDeviceMemory     memory
					offset,                                 // VkDeviceSize       offset
					VK_WHOLE_SIZE                           // VkDeviceSize       size
				}
			};

			vkFlushMappedMemoryRanges(logical_device, static_cast<uint32_t>(memory_ranges.size()), memory_ranges.data());
			if (VK_SUCCESS != result) {
				std::cout << "Could not flush mapped memory." << std::endl;
				return false;
			}

			if (unmap) {
				vkUnmapMemory(logical_device, memory_object);
			}
			else if (nullptr != pointer) {
				*pointer = local_pointer;
			}

			return true;
		}

		void CopyDataBetweenBuffers(VkCommandBuffer           command_buffer,
			VkBuffer                  source_buffer,
			VkBuffer                  destination_buffer,
			std::vector<VkBufferCopy> regions) {
			if (regions.size() > 0) {
				vkCmdCopyBuffer(command_buffer, source_buffer, destination_buffer, static_cast<uint32_t>(regions.size()), regions.data());
			}
		}

		void CopyDataFromBufferToImage(VkCommandBuffer                command_buffer,
			VkBuffer                       source_buffer,
			VkImage                        destination_image,
			VkImageLayout                  image_layout,
			std::vector<VkBufferImageCopy> regions) {
			if (regions.size() > 0) {
				vkCmdCopyBufferToImage(command_buffer, source_buffer, destination_image, image_layout, static_cast<uint32_t>(regions.size()), regions.data());
			}
		}

		void CopyDataFromImageToBuffer(VkCommandBuffer                command_buffer,
			VkImage                        source_image,
			VkImageLayout                  image_layout,
			VkBuffer                       destination_buffer,
			std::vector<VkBufferImageCopy> regions) {
			if (regions.size() > 0) {
				vkCmdCopyImageToBuffer(command_buffer, source_image, image_layout, destination_buffer, static_cast<uint32_t>(regions.size()), regions.data());
			}
		}

		bool UseStagingBufferToUpdateBufferWithDeviceLocalMemoryBound(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkDeviceSize               data_size,
			void                     * data,
			VkBuffer                   destination_buffer,
			VkDeviceSize               destination_offset,
			VkAccessFlags              destination_buffer_current_access,
			VkAccessFlags              destination_buffer_new_access,
			VkPipelineStageFlags       destination_buffer_generating_stages,
			VkPipelineStageFlags       destination_buffer_consuming_stages,
			VkQueue                    queue,
			VkCommandBuffer            command_buffer,
			std::vector<VkSemaphore>   signal_semaphores) {

			VkDestroyer(VkBuffer) staging_buffer;
			InitVkDestroyer(logical_device, staging_buffer);
			if (!CreateBuffer(logical_device, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *staging_buffer)) {
				return false;
			}

			VkDestroyer(VkDeviceMemory) memory_object;
			InitVkDestroyer(logical_device, memory_object);
			if (!AllocateAndBindMemoryObjectToBuffer(physical_device, logical_device, *staging_buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *memory_object)) {
				return false;
			}

			if (!MapUpdateAndUnmapHostVisibleMemory(logical_device, *memory_object, 0, data_size, data, true, nullptr)) {
				return false;
			}

			if (!BeginCommandBufferRecordingOperation(command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr)) {
				return false;
			}

			SetBufferMemoryBarrier(command_buffer, destination_buffer_generating_stages, VK_PIPELINE_STAGE_TRANSFER_BIT, { { destination_buffer, destination_buffer_current_access, VK_ACCESS_TRANSFER_WRITE_BIT, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

			CopyDataBetweenBuffers(command_buffer, *staging_buffer, destination_buffer, { { 0, destination_offset, data_size } });

			SetBufferMemoryBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, destination_buffer_consuming_stages, { { destination_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, destination_buffer_new_access, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED } });

			if (!EndCommandBufferRecordingOperation(command_buffer)) {
				return false;
			}

			VkDestroyer(VkFence) fence;
			InitVkDestroyer(logical_device, fence);
			if (!CreateFence(logical_device, false, *fence)) {
				return false;
			}

			if (!SubmitCommandBuffersToQueue(queue, {}, { command_buffer }, signal_semaphores, *fence)) {
				return false;
			}

			if (!WaitForFences(logical_device, { *fence }, VK_FALSE, 500000000)) {
				return false;
			}

			return true;
		}


		void FreeMemoryObject(VkDevice         logical_device,
			VkDeviceMemory & memory_object) {
			if (VK_NULL_HANDLE != memory_object) {
				vkFreeMemory(logical_device, memory_object, nullptr);
				memory_object = VK_NULL_HANDLE;
			}
		}

		
	}
} // namespace LavaCake

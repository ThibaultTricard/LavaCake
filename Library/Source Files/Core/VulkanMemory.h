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


#ifndef VULKAN_MEMORY
#define VULKAN_MEMORY

#include "Common.h"
#include "VulkanCommand.h"

namespace LavaCake {
	namespace Core {
		bool MapUpdateAndUnmapHostVisibleMemory(VkDevice             logical_device,
			VkDeviceMemory       memory_object,
			VkDeviceSize         offset,
			VkDeviceSize         data_size,
			void               * data,
			bool                 unmap,
			void             * * pointer);

		void CopyDataBetweenBuffers(VkCommandBuffer           command_buffer,
			VkBuffer                  source_buffer,
			VkBuffer                  destination_buffer,
			std::vector<VkBufferCopy> regions);

		void CopyDataFromBufferToImage(VkCommandBuffer                command_buffer,
			VkBuffer                       source_buffer,
			VkImage                        destination_image,
			VkImageLayout                  image_layout,
			std::vector<VkBufferImageCopy> regions);

		void CopyDataFromImageToBuffer(VkCommandBuffer                command_buffer,
			VkImage                        source_image,
			VkImageLayout                  image_layout,
			VkBuffer                       destination_buffer,
			std::vector<VkBufferImageCopy> regions);

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
			std::vector<VkSemaphore>   signal_semaphores);

		bool UseStagingBufferToUpdateImageWithDeviceLocalMemoryBound(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkDeviceSize               data_size,
			void                     * data,
			VkImage                    destination_image,
			VkImageSubresourceLayers   destination_image_subresource,
			VkOffset3D                 destination_image_offset,
			VkExtent3D                 destination_image_size,
			VkImageLayout              destination_image_current_layout,
			VkImageLayout              destination_image_new_layout,
			VkAccessFlags              destination_image_current_access,
			VkAccessFlags              destination_image_new_access,
			VkImageAspectFlags         destination_image_aspect,
			VkPipelineStageFlags       destination_image_generating_stages,
			VkPipelineStageFlags       destination_image_consuming_stages,
			VkQueue                    queue,
			VkCommandBuffer            command_buffer,
			std::vector<VkSemaphore>   signal_semaphores);

		void FreeMemoryObject(VkDevice         logical_device,
			VkDeviceMemory & memory_object);
	}
} // namespace LavaCake

#endif // CREATING_A_BUFFER
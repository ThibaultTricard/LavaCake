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

#ifndef VULKAN_IMAGE
#define VULKAN_IMAGE

#include "Common.h"
#include "VulkanCommand.h"

namespace LavaCake {
	namespace Core {

		struct ImageTransition {
			VkImage             Image;
			VkAccessFlags       CurrentAccess;
			VkAccessFlags       NewAccess;
			VkImageLayout       CurrentLayout;
			VkImageLayout       NewLayout;
			uint32_t            CurrentQueueFamily;
			uint32_t            NewQueueFamily;
			VkImageAspectFlags  Aspect;
		};

		bool CreateImage(VkDevice                logical_device,
			VkImageType             type,
			VkFormat                format,
			VkExtent3D              size,
			uint32_t                num_mipmaps,
			uint32_t                num_layers,
			VkSampleCountFlagBits   samples,
			VkImageUsageFlags       usage_scenarios,
			bool                    cubemap,
			VkImage               & image);

		bool AllocateAndBindMemoryObjectToImage(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkImage                    image,
			VkMemoryPropertyFlagBits   memory_properties,
			VkDeviceMemory           & memory_object);


		bool CreateImageView(VkDevice             logical_device,
			VkImage              image,
			VkImageViewType      view_type,
			VkFormat             format,
			VkImageAspectFlags   aspect,
			VkImageView        & image_view);

		
	}
}

#endif

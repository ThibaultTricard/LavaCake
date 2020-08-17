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



#ifndef VULKAN_RENDERPASS
#define VULKAN_RENDERPASS

#include "Common.h"
#include "VulkanMemory.h"

namespace LavaCake {
	namespace RenderPass {
		struct SubpassParameters {
			VkPipelineBindPoint                  PipelineType;
			std::vector<VkAttachmentReference>   InputAttachments;
			std::vector<VkAttachmentReference>   ColorAttachments;
			std::vector<VkAttachmentReference>   ResolveAttachments;
			VkAttachmentReference const        * DepthStencilAttachment;
			std::vector<uint32_t>                PreserveAttachments;
		};

		void SpecifySubpassDescriptions(std::vector<SubpassParameters> const & subpass_parameters,
			std::vector<VkSubpassDescription>    & subpass_descriptions);

		void SpecifyDependenciesBetweenSubpasses(std::vector<VkSubpassDependency> const & subpasses_dependencies);

		bool CreateRenderPass(VkDevice                                     logical_device,
			std::vector<VkAttachmentDescription> const & attachments_descriptions,
			std::vector<SubpassParameters> const       & subpass_parameters,
			std::vector<VkSubpassDependency> const     & subpass_dependencies,
			VkRenderPass                               & render_pass);

		bool PrepareRenderPassForGeometryRenderingAndPostprocessSubpasses(VkDevice     logical_device,
			VkRenderPass render_pass);

		bool PrepareRenderPassAndFramebufferWithColorAndDepthAttachments(VkPhysicalDevice        physical_device,
			VkDevice                logical_device,
			uint32_t                width,
			uint32_t                height,
			VkImage               & color_image,
			VkDeviceMemory        & color_image_memory_object,
			VkImageView           & color_image_view,
			VkImage               & depth_image,
			VkDeviceMemory        & depth_image_memory_object,
			VkImageView           & depth_image_view,
			VkRenderPass          & render_pass,
			VkFramebuffer         & framebuffer);

		void BeginRenderPass(VkCommandBuffer                   command_buffer,
			VkRenderPass                      render_pass,
			VkFramebuffer                     framebuffer,
			VkRect2D                          render_area,
			std::vector<VkClearValue> const & clear_values,
			VkSubpassContents                 subpass_contents);

		void ProgressToTheNextSubpass(VkCommandBuffer   command_buffer,
			VkSubpassContents subpass_contents);

		void EndRenderPass(VkCommandBuffer command_buffer);

		void DestroyRenderPass(VkDevice       logical_device,
			VkRenderPass & render_pass);

	}
} // namespace LavaCake

#endif // SPECIFYING_ATTACHMENTS_DESCRIPTIONS
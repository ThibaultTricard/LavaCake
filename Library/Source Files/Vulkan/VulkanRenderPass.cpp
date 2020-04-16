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
//
// Chapter: 06 Render Passes and Framebuffers
// Recipe:  01 Specifying attachments descriptions

#include "VulkanRenderPass.h"
#include "VulkanImage.h"
#include "VulkanBuffer.h"

namespace LavaCake {
	namespace RenderPass {
		void SpecifySubpassDescriptions(std::vector<SubpassParameters> const & subpass_parameters,
			std::vector<VkSubpassDescription>    & subpass_descriptions) {
			subpass_descriptions.clear();

			for (auto & subpass_description : subpass_parameters) {
				subpass_descriptions.push_back({
					0,                                                                      // VkSubpassDescriptionFlags        flags
					subpass_description.PipelineType,                                       // VkPipelineBindPoint              pipelineBindPoint
					static_cast<uint32_t>(subpass_description.InputAttachments.size()),     // uint32_t                         inputAttachmentCount
					subpass_description.InputAttachments.data(),                            // const VkAttachmentReference    * pInputAttachments
					static_cast<uint32_t>(subpass_description.ColorAttachments.size()),     // uint32_t                         colorAttachmentCount
					subpass_description.ColorAttachments.data(),                            // const VkAttachmentReference    * pColorAttachments
					subpass_description.ResolveAttachments.data(),                          // const VkAttachmentReference    * pResolveAttachments
					subpass_description.DepthStencilAttachment,                             // const VkAttachmentReference    * pDepthStencilAttachment
					static_cast<uint32_t>(subpass_description.PreserveAttachments.size()),  // uint32_t                         preserveAttachmentCount
					subpass_description.PreserveAttachments.data()                          // const uint32_t                 * pPreserveAttachments
					});
			}
		}

		void SpecifyDependenciesBetweenSubpasses(std::vector<VkSubpassDependency> const & subpasses_dependencies) {
			// typedef struct VkSubpassDependency {
			//   uint32_t                srcSubpass;
			//   uint32_t                dstSubpass;
			//   VkPipelineStageFlags    srcStageMask;
			//   VkPipelineStageFlags    dstStageMask;
			//   VkAccessFlags           srcAccessMask;
			//   VkAccessFlags           dstAccessMask;
			//   VkDependencyFlags       dependencyFlags;
			// } VkSubpassDependency;
		}

		bool PrepareRenderPassForGeometryRenderingAndPostprocessSubpasses(VkDevice     logical_device,
			VkRenderPass render_pass) {
			std::vector<VkAttachmentDescription> attachments_descriptions = {
				{
					0,                                                // VkAttachmentDescriptionFlags     flags
					VK_FORMAT_R8G8B8A8_UNORM,                         // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         // VkImageLayout                    finalLayout
				},
				{
					0,                                                // VkAttachmentDescriptionFlags     flags
					VK_FORMAT_D16_UNORM,                              // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // VkImageLayout                    finalLayout
				},
				{
					0,                                                // VkAttachmentDescriptionFlags     flags
					VK_FORMAT_R8G8B8A8_UNORM,                         // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_STORE,                     // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                  // VkImageLayout                    finalLayout
				},
			};

			VkAttachmentReference depth_stencil_attachment = {
				1,                                                  // uint32_t                             attachment
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout                        layout
			};

			std::vector<SubpassParameters> subpass_parameters = {
				// #0 subpass
				{
					VK_PIPELINE_BIND_POINT_GRAPHICS,                  // VkPipelineBindPoint                  PipelineType
					{},                                               // std::vector<VkAttachmentReference>   InputAttachments
					{                                                 // std::vector<VkAttachmentReference>   ColorAttachments
						{
							0,                                              // uint32_t                             attachment
							VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        // VkImageLayout                        layout
						}
					},
					{},                                               // std::vector<VkAttachmentReference>   ResolveAttachments
					&depth_stencil_attachment,                        // const VkAttachmentReference        * DepthStencilAttachment
					{}                                                // std::vector<uint32_t>                PreserveAttachments
				},
				// #1 subpass
				{
					VK_PIPELINE_BIND_POINT_GRAPHICS,                  // VkPipelineBindPoint                  PipelineType
					{                                                 // std::vector<VkAttachmentReference>   InputAttachments
						{
							0,                                              // uint32_t                             attachment
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL        // VkImageLayout                        layout
						}
					},
					{                                                 // std::vector<VkAttachmentReference>   ColorAttachments
						{
							2,                                              // uint32_t                             attachment
							VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        // VkImageLayout                        layout
						}
					},
					{},                                               // std::vector<VkAttachmentReference>   ResolveAttachments
					nullptr,                                          // const VkAttachmentReference        * DepthStencilAttachment
					{}                                                // std::vector<uint32_t>                PreserveAttachments
				}
			};

			std::vector<VkSubpassDependency> subpass_dependencies = {
				{
					0,                                              // uint32_t                 srcSubpass
					1,                                              // uint32_t                 dstSubpass
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // VkPipelineStageFlags     srcStageMask
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,          // VkPipelineStageFlags     dstStageMask
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,           // VkAccessFlags            srcAccessMask
					VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,            // VkAccessFlags            dstAccessMask
					VK_DEPENDENCY_BY_REGION_BIT                     // VkDependencyFlags        dependencyFlags
				}
			};

			if (!CreateRenderPass(logical_device, attachments_descriptions, subpass_parameters, subpass_dependencies, render_pass)) {
				return false;
			}
			return true;
		}

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
			VkFramebuffer         & framebuffer) {
			if (!Image::Create2DImageAndView(physical_device, logical_device, VK_FORMAT_R8G8B8A8_UNORM, { width, height }, 1, 1, VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, color_image, color_image_memory_object, color_image_view)) {
				return false;
			}

			if (!Image::Create2DImageAndView(physical_device, logical_device, VK_FORMAT_D16_UNORM, { width, height }, 1, 1, VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, depth_image, depth_image_memory_object, depth_image_view)) {
				return false;
			}

			std::vector<VkAttachmentDescription> attachments_descriptions = {
				{
					0,                                                // VkAttachmentDescriptionFlags     flags
					VK_FORMAT_R8G8B8A8_UNORM,                         // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_STORE,                     // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,         // VkImageLayout                    finalLayout
				},
				{
					0,                                                // VkAttachmentDescriptionFlags     flags
					VK_FORMAT_D16_UNORM,                              // VkFormat                         format
					VK_SAMPLE_COUNT_1_BIT,                            // VkSampleCountFlagBits            samples
					VK_ATTACHMENT_LOAD_OP_CLEAR,                      // VkAttachmentLoadOp               loadOp
					VK_ATTACHMENT_STORE_OP_STORE,                     // VkAttachmentStoreOp              storeOp
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,                  // VkAttachmentLoadOp               stencilLoadOp
					VK_ATTACHMENT_STORE_OP_DONT_CARE,                 // VkAttachmentStoreOp              stencilStoreOp
					VK_IMAGE_LAYOUT_UNDEFINED,                        // VkImageLayout                    initialLayout
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  // VkImageLayout                    finalLayout
				}
			};

			VkAttachmentReference depth_stencil_attachment = {
				1,                                                  // uint32_t                             attachment
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,   // VkImageLayout                        layout
			};

			std::vector<SubpassParameters> subpass_parameters = {
				{
					VK_PIPELINE_BIND_POINT_GRAPHICS,                  // VkPipelineBindPoint                  PipelineType
					{},                                               // std::vector<VkAttachmentReference>   InputAttachments
					{                                                 // std::vector<VkAttachmentReference>   ColorAttachments
						{
							0,                                              // uint32_t                             attachment
							VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL        // VkImageLayout                        layout
						}
					},
					{},                                               // std::vector<VkAttachmentReference>   ResolveAttachments
					&depth_stencil_attachment,                        // const VkAttachmentReference        * DepthStencilAttachment
					{}                                                // std::vector<uint32_t>                PreserveAttachments
				}
			};

			std::vector<VkSubpassDependency> subpass_dependencies = {
				{
					0,                                                // uint32_t                 srcSubpass
					VK_SUBPASS_EXTERNAL,                              // uint32_t                 dstSubpass
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,    // VkPipelineStageFlags     srcStageMask
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,            // VkPipelineStageFlags     dstStageMask
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,             // VkAccessFlags            srcAccessMask
					VK_ACCESS_SHADER_READ_BIT,                        // VkAccessFlags            dstAccessMask
					0                                                 // VkDependencyFlags        dependencyFlags
				}
			};

			if (!CreateRenderPass(logical_device, attachments_descriptions, subpass_parameters, subpass_dependencies, render_pass)) {
				return false;
			}

			if (!Buffer::CreateFramebuffer(logical_device, render_pass, { color_image_view, depth_image_view }, width, height, 1, framebuffer)) {
				return false;
			}
			return true;
		}

		bool CreateRenderPass(VkDevice                                     logical_device,
			std::vector<VkAttachmentDescription> const & attachments_descriptions,
			std::vector<SubpassParameters> const       & subpass_parameters,
			std::vector<VkSubpassDependency> const     & subpass_dependencies,
			VkRenderPass                               & render_pass) {

			Image::SpecifyAttachmentsDescriptions(attachments_descriptions);

			std::vector<VkSubpassDescription> subpass_descriptions;
			SpecifySubpassDescriptions(subpass_parameters, subpass_descriptions);

			SpecifyDependenciesBetweenSubpasses(subpass_dependencies);

			VkRenderPassCreateInfo render_pass_create_info = {
				VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                // VkStructureType                    sType
				nullptr,                                                  // const void                       * pNext
				0,                                                        // VkRenderPassCreateFlags            flags
				static_cast<uint32_t>(attachments_descriptions.size()),   // uint32_t                           attachmentCount
				attachments_descriptions.data(),                          // const VkAttachmentDescription    * pAttachments
				static_cast<uint32_t>(subpass_descriptions.size()),       // uint32_t                           subpassCount
				subpass_descriptions.data(),                              // const VkSubpassDescription       * pSubpasses
				static_cast<uint32_t>(subpass_dependencies.size()),       // uint32_t                           dependencyCount
				subpass_dependencies.data()                               // const VkSubpassDependency        * pDependencies
			};

			VkResult result = vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass);
			if (VK_SUCCESS != result) {
				std::cout << "Could not create a render pass." << std::endl;
				return false;
			}
			return true;
		}




		void BeginRenderPass(VkCommandBuffer                   command_buffer,
			VkRenderPass                      render_pass,
			VkFramebuffer                     framebuffer,
			VkRect2D                          render_area,
			std::vector<VkClearValue> const & clear_values,
			VkSubpassContents                 subpass_contents) {
			VkRenderPassBeginInfo render_pass_begin_info = {
				VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,     // VkStructureType        sType
				nullptr,                                      // const void           * pNext
				render_pass,                                  // VkRenderPass           renderPass
				framebuffer,                                  // VkFramebuffer          framebuffer
				render_area,                                  // VkRect2D               renderArea
				static_cast<uint32_t>(clear_values.size()),   // uint32_t               clearValueCount
				clear_values.data()                           // const VkClearValue   * pClearValues
			};

			vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, subpass_contents);
		}

		void ProgressToTheNextSubpass(VkCommandBuffer   command_buffer,
			VkSubpassContents subpass_contents) {
			vkCmdNextSubpass(command_buffer, subpass_contents);
		}

		void EndRenderPass(VkCommandBuffer command_buffer) {
			vkCmdEndRenderPass(command_buffer);
		}

		void DestroyRenderPass(VkDevice       logical_device,
			VkRenderPass & render_pass) {
			if (VK_NULL_HANDLE != render_pass) {
				vkDestroyRenderPass(logical_device, render_pass, nullptr);
				render_pass = VK_NULL_HANDLE;
			}
		}
	}

} // namespace LavaCake

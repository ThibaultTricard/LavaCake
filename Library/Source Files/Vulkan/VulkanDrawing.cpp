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
// Chapter: 09 Command Recording and Drawing
// Recipe:  01 Clearing a color image

#include "VulkanDrawing.h"
#include "VulkanFence.h"
#include "VulkanSwapchain.h"

namespace LavaCake {
	namespace Drawing {
		void ProvideDataToShadersThroughPushConstants(VkCommandBuffer      command_buffer,
			VkPipelineLayout     pipeline_layout,
			VkShaderStageFlags   pipeline_stages,
			uint32_t             offset,
			uint32_t             size,
			void               * data) {
			vkCmdPushConstants(command_buffer, pipeline_layout, pipeline_stages, offset, size, data);
		}

		void SetLineWidthStateDynamically(VkCommandBuffer command_buffer,
			float           line_width) {
			vkCmdSetLineWidth(command_buffer, line_width);
		}

		void SetDepthBiasStateDynamically(VkCommandBuffer command_buffer,
			float           constant_factor,
			float           clamp,
			float           slope_factor) {
			vkCmdSetDepthBias(command_buffer, constant_factor, clamp, slope_factor);
		}

		void SetBlendConstantsStateDynamically(VkCommandBuffer              command_buffer,
			std::array<float, 4> const & blend_constants) {
			vkCmdSetBlendConstants(command_buffer, blend_constants.data());
		}

		void DrawGeometry(VkCommandBuffer command_buffer,
			uint32_t        vertex_count,
			uint32_t        instance_count,
			uint32_t        first_vertex,
			uint32_t        first_instance) {
			vkCmdDraw(command_buffer, vertex_count, instance_count, first_vertex, first_instance);
		}

		void DrawIndexedGeometry(VkCommandBuffer command_buffer,
			uint32_t        index_count,
			uint32_t        instance_count,
			uint32_t        first_index,
			uint32_t        vertex_offset,
			uint32_t        first_instance) {
			vkCmdDrawIndexed(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
		}


		bool PrepareSingleFrameOfAnimation(VkDevice                                                        logical_device,
			VkQueue                                                         graphics_queue,
			VkQueue                                                         present_queue,
			VkSwapchainKHR                                                  swapchain,
			VkExtent2D                                                      swapchain_size,
			std::vector<VkImageView> const                                & swapchain_image_views,
			VkImageView                                                     depth_attachment,
			std::vector<Semaphore::WaitSemaphoreInfo> const                          & wait_infos,
			VkSemaphore                                                     image_acquired_semaphore,
			VkSemaphore                                                     ready_to_present_semaphore,
			VkFence                                                         finished_drawing_fence,
			std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)>   record_command_buffer,
			VkCommandBuffer                                                 command_buffer,
			VkRenderPass                                                    render_pass,
			VkDestroyer(VkFramebuffer)                                    & framebuffer) {
			uint32_t image_index;
			if (!Swapchain::AcquireSwapchainImage(logical_device, swapchain, image_acquired_semaphore, VK_NULL_HANDLE, image_index)) {
				return false;
			}

			std::vector<VkImageView> attachments = { swapchain_image_views[image_index] };
			if (VK_NULL_HANDLE != depth_attachment) {
				attachments.push_back(depth_attachment);
			}
			if (!Buffer::CreateFramebuffer(logical_device, render_pass, attachments, swapchain_size.width, swapchain_size.height, 1, *framebuffer)) {
				return false;
			}

			if (!record_command_buffer(command_buffer, image_index, *framebuffer)) {
				return false;
			}

			std::vector<Semaphore::WaitSemaphoreInfo> wait_semaphore_infos = wait_infos;
			wait_semaphore_infos.push_back({
				image_acquired_semaphore,                     // VkSemaphore            Semaphore
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT // VkPipelineStageFlags   WaitingStage
				});
			if (!Command::SubmitCommandBuffersToQueue(graphics_queue, wait_semaphore_infos, { command_buffer }, { ready_to_present_semaphore }, finished_drawing_fence)) {
				return false;
			}

			Presentation::PresentInfo present_info = {
				swapchain,                                    // VkSwapchainKHR         Swapchain
				image_index                                   // uint32_t               ImageIndex
			};
			if (!Presentation::PresentImage(present_queue, { ready_to_present_semaphore }, { present_info })) {
				return false;
			}
			return true;
		}

		bool IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(VkDevice                                                        logical_device,
			VkQueue                                                         graphics_queue,
			VkQueue                                                         present_queue,
			VkSwapchainKHR                                                  swapchain,
			VkExtent2D                                                      swapchain_size,
			std::vector<VkImageView> const                                & swapchain_image_views,
			VkRenderPass                                                    render_pass,
			std::vector<Semaphore::WaitSemaphoreInfo> const                          & wait_infos,
			std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)>   record_command_buffer,
			std::vector<Buffer::FrameResources>                                   & frame_resources) {
			static uint32_t frame_index = 0;
			Buffer::FrameResources & current_frame = frame_resources[frame_index];

			if (!Fence::WaitForFences(logical_device, { *current_frame.DrawingFinishedFence }, false, 2000000000)) {
				return false;
			}
			if (!Fence::ResetFences(logical_device, { *current_frame.DrawingFinishedFence })) {
				return false;
			}

			InitVkDestroyer(logical_device, current_frame.Framebuffer);

			if (!PrepareSingleFrameOfAnimation(logical_device, graphics_queue, present_queue, swapchain, swapchain_size, swapchain_image_views,
				*current_frame.DepthAttachment, wait_infos, *current_frame.ImageAcquiredSemaphore, *current_frame.ReadyToPresentSemaphore,
				*current_frame.DrawingFinishedFence, record_command_buffer, current_frame.CommandBuffer, render_pass, current_frame.Framebuffer)) {
				return false;
			}

			frame_index = (frame_index + 1) % frame_resources.size();
			return true;
		}
	}
} // namespace LavaCake

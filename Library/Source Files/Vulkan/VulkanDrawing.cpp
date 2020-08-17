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



#include "VulkanDrawing.h"
#include "VulkanFence.h"
#include "VulkanSwapchain.h"

namespace LavaCake {
	namespace Drawing {


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

			if (!Fence::WaitForFences(logical_device, { *current_frame.drawingFinishedFence }, false, 2000000000)) {
				return false;
			}
			if (!Fence::ResetFences(logical_device, { *current_frame.drawingFinishedFence })) {
				return false;
			}

			InitVkDestroyer(logical_device, current_frame.framebuffer);

			if (!PrepareSingleFrameOfAnimation(logical_device, graphics_queue, present_queue, swapchain, swapchain_size, swapchain_image_views,
				*current_frame.depthAttachment, wait_infos, *current_frame.imageAcquiredSemaphore, *current_frame.readyToPresentSemaphore,
				*current_frame.drawingFinishedFence, record_command_buffer, current_frame.commandBuffer, render_pass, current_frame.framebuffer)) {
				return false;
			}

			frame_index = (frame_index + 1) % frame_resources.size();
			return true;
		}
	}
} // namespace LavaCake

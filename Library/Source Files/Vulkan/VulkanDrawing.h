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
// Chapter: 09 Command Recording and Drawing
// Recipe:  01 Clearing a color image

#ifndef VULKAN_DRAWING
#define VULKAN_DRAWING

#include "Common.h"
#include "Helpers/helpers.h"
#include "VulkanCommand.h"
#include "VulkanLoader.h"
#include "VulkanRenderPass.h"
#include "VulkanPresentation.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"

namespace LavaCake {
	namespace Drawing {
		void ProvideDataToShadersThroughPushConstants(
			VkCommandBuffer      command_buffer,
			VkPipelineLayout     pipeline_layout,
			VkShaderStageFlags   pipeline_stages,
			uint32_t             offset,
			uint32_t             size,
			void               * data);

		void SetLineWidthStateDynamically(
			VkCommandBuffer command_buffer,
			float           line_width);

		void SetDepthBiasStateDynamically(
			VkCommandBuffer command_buffer,
			float           constant_factor,
			float           clamp_value,
			float           slope_factor);

		void SetBlendConstantsStateDynamically(
			VkCommandBuffer              command_buffer,
			std::array<float, 4> const & blend_constants);

		void DrawGeometry(
			VkCommandBuffer command_buffer,
			uint32_t        vertex_count,
			uint32_t        instance_count,
			uint32_t        first_vertex,
			uint32_t        first_instance);

		void DrawIndexedGeometry(
			VkCommandBuffer command_buffer,
			uint32_t        index_count,
			uint32_t        instance_count,
			uint32_t        first_index,
			uint32_t        vertex_offset,
			uint32_t        first_instance);

		bool PrepareSingleFrameOfAnimation(
			VkDevice                                                        logical_device,
			VkQueue                                                         graphics_queue,
			VkQueue                                                         present_queue,
			VkSwapchainKHR                                                  swapchain,
			VkExtent2D                                                      swapchain_size,
			std::vector<VkImageView> const                                & swapchain_image_views,
			VkImageView                                                     depth_attachment,
			std::vector<Semaphore::WaitSemaphoreInfo> const               & wait_infos,
			VkSemaphore                                                     image_acquired_semaphore,
			VkSemaphore                                                     ready_to_present_semaphore,
			VkFence                                                         finished_drawing_fence,
			std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)>   record_command_buffer,
			VkCommandBuffer                                                 command_buffer,
			VkRenderPass                                                    render_pass,
			VkDestroyer(VkFramebuffer)                                    & framebuffer);


		bool IncreasePerformanceThroughIncreasingTheNumberOfSeparatelyRenderedFrames(
			VkDevice                                                        logical_device,
			VkQueue                                                         graphics_queue,
			VkQueue                                                         present_queue,
			VkSwapchainKHR                                                  swapchain,
			VkExtent2D                                                      swapchain_size,
			std::vector<VkImageView> const                                & swapchain_image_views,
			VkRenderPass                                                    render_pass,
			std::vector<Semaphore::WaitSemaphoreInfo> const               & wait_infos,
			std::function<bool(VkCommandBuffer, uint32_t, VkFramebuffer)>   record_command_buffer,
			std::vector < Buffer::FrameResources > & frame_resources);
	}
} // namespace LavaCake

#endif // CLEARING_A_COLOR_IMAGE
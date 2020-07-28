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

#ifndef VULKAN_BUFFER
#define VULKAN_BUFFER

#include "Common.h"
#include "VulkanMemory.h"
#include "Helpers/helpers.h"

namespace LavaCake {
	
	namespace Buffer {


		struct BufferDescriptorInfo {
			VkDescriptorSet                     TargetDescriptorSet;
			uint32_t                            TargetDescriptorBinding;
			uint32_t                            TargetArrayElement;
			VkDescriptorType                    TargetDescriptorType;
			std::vector<VkDescriptorBufferInfo> BufferInfos;
		};

		struct TexelBufferDescriptorInfo {
			VkDescriptorSet                     TargetDescriptorSet;
			uint32_t                            TargetDescriptorBinding;
			uint32_t                            TargetArrayElement;
			VkDescriptorType                    TargetDescriptorType;
			std::vector<VkBufferView>           TexelBufferViews;
		};

		struct BufferTransition {
			VkBuffer        Buffer;
			VkAccessFlags   CurrentAccess;
			VkAccessFlags   NewAccess;
			uint32_t        CurrentQueueFamily;
			uint32_t        NewQueueFamily;
		};

		struct FrameResources {
			VkCommandBuffer             commandBuffer;
			VkDestroyer(VkSemaphore)    imageAcquiredSemaphore;
			VkDestroyer(VkSemaphore)    readyToPresentSemaphore;
			VkDestroyer(VkFence)        drawingFinishedFence;
			VkDestroyer(VkImageView)    depthAttachment;
			VkDestroyer(VkFramebuffer)  framebuffer;

			FrameResources(VkCommandBuffer            & command_buffer,
				VkDestroyer(VkSemaphore)   & image_acquired_semaphore,
				VkDestroyer(VkSemaphore)   & ready_to_present_semaphore,
				VkDestroyer(VkFence)       & drawing_finished_fence,
				VkDestroyer(VkImageView)   & depth_attachment,
				VkDestroyer(VkFramebuffer) & framebuffer) :
				commandBuffer(command_buffer),
				imageAcquiredSemaphore(std::move(image_acquired_semaphore)),
				readyToPresentSemaphore(std::move(ready_to_present_semaphore)),
				drawingFinishedFence(std::move(drawing_finished_fence)),
				depthAttachment(std::move(depth_attachment)),
				framebuffer(std::move(framebuffer)) {
			}

			FrameResources(FrameResources && other) {
				*this = std::move(other);
			}

			FrameResources& operator=(FrameResources && other) {
				if (this != &other) {
					VkCommandBuffer command_buffer = commandBuffer;

					commandBuffer = other.commandBuffer;
					other.commandBuffer = command_buffer;
					imageAcquiredSemaphore = std::move(other.imageAcquiredSemaphore);
					readyToPresentSemaphore = std::move(other.readyToPresentSemaphore);
					drawingFinishedFence = std::move(other.drawingFinishedFence);
					depthAttachment = std::move(other.depthAttachment);
					framebuffer = std::move(other.framebuffer);
				}
				return *this;
			}

			FrameResources(FrameResources const &) = delete;
			FrameResources& operator=(FrameResources const &) = delete;
		};

		struct VertexBufferParameters {
			VkBuffer      Buffer;
			VkDeviceSize  MemoryOffset;
		};

		struct CommandBufferRecordingThreadParameters {
			VkCommandBuffer                         CommandBuffer;
			std::function<bool(VkCommandBuffer)>  RecordingFunction;
		};

		bool CreateBuffer(VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & buffer);

		bool AllocateAndBindMemoryObjectToBuffer(VkPhysicalDevice           physical_device,
			VkDevice                   logical_device,
			VkBuffer                   buffer,
			VkMemoryPropertyFlagBits   memory_properties,
			VkDeviceMemory           & memory_object);

		void SetBufferMemoryBarrier(VkCommandBuffer               command_buffer,
			VkPipelineStageFlags          generating_stages,
			VkPipelineStageFlags          consuming_stages,
			std::vector<BufferTransition> buffer_transitions);

		bool CreateBufferView(VkDevice       logical_device,
			VkBuffer       buffer,
			VkFormat       format,
			VkDeviceSize   memory_offset,
			VkDeviceSize   memory_range,
			VkBufferView & buffer_view);

		void DestroyBufferView(VkDevice       logical_device,
			VkBufferView & buffer_view);

		void ClearColorImage(VkCommandBuffer                              command_buffer,
			VkImage                                      image,
			VkImageLayout                                image_layout,
			std::vector<VkImageSubresourceRange> const & image_subresource_ranges,
			VkClearColorValue                          & clear_color);

		void ClearDepthStencilImage(VkCommandBuffer                              command_buffer,
			VkImage                                      image,
			VkImageLayout                                image_layout,
			std::vector<VkImageSubresourceRange> const & image_subresource_ranges,
			VkClearDepthStencilValue                   & clear_value);

		void ClearRenderPassAttachments(VkCommandBuffer                        command_buffer,
			std::vector<VkClearAttachment> const & attachments,
			std::vector<VkClearRect> const       & rects);

		void BindVertexBuffers(VkCommandBuffer                             command_buffer,
			uint32_t                                    first_binding,
			std::vector<VertexBufferParameters> const & buffers_parameters);

		void BindIndexBuffer(VkCommandBuffer   command_buffer,
			VkBuffer          buffer,
			VkDeviceSize      memory_offset,
			VkIndexType       index_type);
		

		bool RecordCommandBufferThatDrawsGeometryWithDynamicViewportAndScissorStates(VkCommandBuffer                             command_buffer,
			VkImage                                     swapchain_image,
			uint32_t                                    present_queue_family_index,
			uint32_t                                    graphics_queue_family_index,
			VkRenderPass                                render_pass,
			VkFramebuffer                               framebuffer,
			VkExtent2D                                  framebuffer_size,
			std::vector<VkClearValue> const &           clear_values,
			VkPipeline                                  graphics_pipeline,
			uint32_t                                    first_vertex_buffer_binding,
			std::vector<VertexBufferParameters> const & vertex_buffers_parameters,
			VkPipelineLayout                            pipeline_layout,
			std::vector<VkDescriptorSet> const &        descriptor_sets,
			uint32_t                                    index_for_first_descriptor_set,
			Helpers::Mesh::Mesh const &                  geometry,
			uint32_t                                    instance_count,
			uint32_t                                    first_instance);

		bool RecordCommandBuffersOnMultipleThreads(std::vector<CommandBufferRecordingThreadParameters> const & threads_parameters,
			VkQueue                                                     queue,
			std::vector<Semaphore::WaitSemaphoreInfo>                              wait_semaphore_infos,
			std::vector<VkSemaphore>                                    signal_semaphores,
			VkFence                                                     fence);
	
		bool CreateSampler(VkDevice               logical_device,
			VkFilter               mag_filter,
			VkFilter               min_filter,
			VkSamplerMipmapMode    mipmap_mode,
			VkSamplerAddressMode   u_address_mode,
			VkSamplerAddressMode   v_address_mode,
			VkSamplerAddressMode   w_address_mode,
			float                  lod_bias,
			bool                   anisotropy_enable,
			float                  max_anisotropy,
			bool                   compare_enable,
			VkCompareOp            compare_operator,
			float                  min_lod,
			float                  max_lod,
			VkBorderColor          border_color,
			bool                   unnormalized_coords,
			VkSampler            & sampler);

		

		bool CreateUniformTexelBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkFormat             format,
			VkDeviceSize         size,
			VkImageUsageFlags    usage,
			VkBuffer           & uniform_texel_buffer,
			VkDeviceMemory     & memory_object,
			VkBufferView       & uniform_texel_buffer_view);

		bool CreateStorageTexelBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkFormat             format,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			bool                 atomic_operations,
			VkBuffer           & storage_texel_buffer,
			VkDeviceMemory     & memory_object,
			VkBufferView       & storage_texel_buffer_view);

		bool CreateUniformBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & uniform_buffer,
			VkDeviceMemory     & memory_object);

		bool CreateStorageBuffer(VkPhysicalDevice     physical_device,
			VkDevice             logical_device,
			VkDeviceSize         size,
			VkBufferUsageFlags   usage,
			VkBuffer           & storage_buffer,
			VkDeviceMemory     & memory_object);

		void DestroyBuffer(VkDevice   logical_device,
			VkBuffer & buffer);

		void DestroySampler(VkDevice    logical_device,
			VkSampler & sampler);

		bool CreateFramebuffer(VkDevice                         logical_device,
			VkRenderPass                     render_pass,
			std::vector<VkImageView> const & attachments,
			uint32_t                         width,
			uint32_t                         height,
			uint32_t                         layers,
			VkFramebuffer                  & framebuffer);

		void DestroyFramebuffer(VkDevice        logical_device,
			VkFramebuffer & framebuffer);

	}
}

#endif
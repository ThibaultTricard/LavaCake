#pragma once

#include "CommandBuffer.h"
#include "Queue.h"

namespace LavaCake {
  namespace Framework {

    class Image {
    public :
      Image(uint32_t width, uint32_t height, uint32_t depth, VkFormat f, VkImageAspectFlagBits aspect, bool cubemap = false) {
        m_width = width;
        m_height = height;
        m_depth = depth;
        m_format = f;
				m_aspect = aspect;
				m_cubemap = cubemap;
      }

      void allocate(VkImageUsageFlags usage) {
        Framework::Device* d = LavaCake::Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        InitVkDestroyer(logical, m_image);
        InitVkDestroyer(logical, m_imageMemory);
        InitVkDestroyer(logical, m_imageView);
        VkPhysicalDevice physical = d->getPhysicalDevice();



        VkImageType type = VK_IMAGE_TYPE_1D;
        VkImageViewType view = VK_IMAGE_VIEW_TYPE_1D;
        if (m_height > 1) { type = VK_IMAGE_TYPE_2D; view = VK_IMAGE_VIEW_TYPE_2D; }
        if (m_depth > 1) { type = VK_IMAGE_TYPE_3D; view = VK_IMAGE_VIEW_TYPE_3D; }

        if (!LavaCake::Core::CreateImage(logical, type, m_format, { m_width, m_height, m_depth }, 1, 1, VK_SAMPLE_COUNT_1_BIT, usage, m_cubemap, *m_image)) {
          ErrorCheck::setError("Can't create Image");
        }

        if (!LavaCake::Core::AllocateAndBindMemoryObjectToImage(physical, logical, *m_image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_imageMemory)) {
          ErrorCheck::setError("Can't allocate Image memory");
        }

        if (!LavaCake::Core::CreateImageView(logical, *m_image, view, m_format, m_aspect, *m_imageView)) {
          ErrorCheck::setError("Can't create Image View");
        }

				m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
				m_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      }

      void map() {
				Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				vkMapMemory(logical, *m_imageMemory, 0, VK_WHOLE_SIZE, 0, &m_mappedMemory);
      }

			void unmap() {
				Framework::Device* d = LavaCake::Framework::Device::getDevice();
				VkDevice logical = d->getLogicalDevice();
				vkUnmapMemory(logical, *m_imageMemory);
			}


      void setLayout(CommandBuffer& cmdbuff, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags dstStage) {

					// Create an image barrier object
					VkImageMemoryBarrier imageMemoryBarrier{};
					imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					imageMemoryBarrier.oldLayout = m_layout;
					imageMemoryBarrier.newLayout = newLayout;
					imageMemoryBarrier.image = *m_image;
					imageMemoryBarrier.subresourceRange = subresourceRange;

					// Source layouts (old)
					// Source access mask controls actions that have to be finished on the old layout
					// before it will be transitioned to the new layout
					switch (m_layout)
					{
					case VK_IMAGE_LAYOUT_UNDEFINED:
						// Image layout is undefined (or does not matter)
						// Only valid as initial layout
						// No flags required, listed only for completeness
						imageMemoryBarrier.srcAccessMask = 0;
						break;

					case VK_IMAGE_LAYOUT_PREINITIALIZED:
						// Image is preinitialized
						// Only valid as initial layout for linear images, preserves memory contents
						// Make sure host writes have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
						// Image is a color attachment
						// Make sure any writes to the color buffer have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
						// Image is a depth/stencil attachment
						// Make sure any writes to the depth/stencil buffer have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
						// Image is a transfer source
						// Make sure any reads from the image have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						break;

					case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
						// Image is a transfer destination
						// Make sure any writes to the image have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
						// Image is read by a shader
						// Make sure any shader reads from the image have been finished
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
						break;
					default:
						// Other source layouts aren't handled (yet)
						break;
					}

					// Target layouts (new)
					// Destination access mask controls the dependency for the new image layout
					switch (newLayout)
					{
					case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
						// Image will be used as a transfer destination
						// Make sure any writes to the image have been finished
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
						// Image will be used as a transfer source
						// Make sure any reads from the image have been finished
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						break;

					case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
						// Image will be used as a color attachment
						// Make sure any writes to the color buffer have been finished
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
						// Image layout will be used as a depth/stencil attachment
						// Make sure any writes to depth/stencil buffer have been finished
						imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						break;

					case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
						// Image will be read in a shader (sampler, input attachment)
						// Make sure any writes to the image have been finished
						if (imageMemoryBarrier.srcAccessMask == 0)
						{
							imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
						}
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
						break;
					default:
						// Other source layouts aren't handled (yet)
						break;
					}

					// Put barrier inside setup command buffer
					vkCmdPipelineBarrier(
						cmdbuff.getHandle(),
						m_stage,
						dstStage,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageMemoryBarrier);

					m_layout = newLayout;
					m_stage = dstStage;

      }


			VkImage& getImage() {
				return *m_image;
			}
			VkDeviceMemory& getImageMemory() {
				return *m_imageMemory;
			}

			VkImageView& getImageView() {
				return *m_imageView;
			}


			~Image() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();


				LavaCake::Core::DestroyImage(logical, *m_image);
				LavaCake::Core::DestroyImageView(logical, *m_imageView);
				LavaCake::Core::FreeMemoryObject(logical, *m_imageMemory);
			}

			VkImageLayout getLayout() {
				return m_layout;
			}

			uint32_t width() {
				return m_width;
			}

			uint32_t height() {
				return m_height;
			}

			uint32_t depth() {
				return m_depth;
			}

    private:

      uint32_t														m_width = 0;
      uint32_t														m_height = 0;
      uint32_t														m_depth = 0;
      VkFormat														m_format;

			VkImageLayout												m_layout;
			VkPipelineStageFlags								m_stage;
			VkImageAspectFlagBits								m_aspect;

      VkDestroyer(VkImage)                m_image;
      VkDestroyer(VkDeviceMemory)         m_imageMemory;
      VkDestroyer(VkImageView)            m_imageView;

			bool																m_cubemap;

			void*																m_mappedMemory;
    };

  }
}
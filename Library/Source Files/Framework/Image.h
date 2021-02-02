#pragma once

#include "CommandBuffer.h"
#include "Queue.h"
#include "Buffer.h"



namespace LavaCake {
  namespace Framework {


		class Buffer;

    class Image {
    public :

			Image(uint32_t width, uint32_t height, uint32_t depth, VkFormat f, VkImageAspectFlagBits aspect, bool cubemap = false);

			void allocate(VkImageUsageFlags usage);

      void map();

			void unmap();

			void setLayout(CommandBuffer& cmdbuff, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags dstStage);

			void copyToImage(CommandBuffer& cmdBuff, Image& image, std::vector<VkImageCopy> regions);

			void copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, std::vector<VkBufferImageCopy> regions);

			VkImage& getImage();

			VkDeviceMemory& getImageMemory();

			VkImageView& getImageView();

			~Image() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();


				if (VK_NULL_HANDLE != *m_image) {
					vkDestroyImage(logical, *m_image, nullptr);
					*m_image = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_imageView) {
					vkDestroyImageView(logical, *m_imageView, nullptr);
					*m_imageView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != *m_imageMemory) {
					vkFreeMemory(logical, *m_imageMemory, nullptr);
					*m_imageMemory = VK_NULL_HANDLE;
				}

			}

			VkImageLayout getLayout();

			uint32_t width();

			uint32_t height();

			uint32_t depth();

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
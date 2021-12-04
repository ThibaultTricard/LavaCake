#pragma once

#include "CommandBuffer.h"
#include "Queue.h"
#include "Buffer.h"



namespace LavaCake {
  namespace Framework {


		class Buffer;
  
    /**
     Class Image
     \brief This class helps manage Vulkan Images, their memory and their view
     */
    class Image {
    public :
      
      /**
       \brief Create an image
       \param witdh the witdth of the image
       \param height the height of the image
       \param depth the depth of the image
       \param format the format of the Image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFormat.html">here</a>
       \param aspect the aspect of the image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageAspectFlagBits.html">here</a>
       \param cubemap [otpional] weither or not the image is a cubemap, false by defalt
       */
			Image(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageAspectFlagBits aspect, bool cubemap = false);

      /**
       \brief allocate the image on the GPU
       \param usage the usage of the image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageUsageFlags.html">here</a>
       \param memPropertyFlag : the memory property of the image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryPropertyFlagBits.html">here</a>
       */
			void allocate(VkImageUsageFlags usage,
                    VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      /**
       \brief Map the memory of the Image
       \warning {The image must have been allocated with the memory flag bits set to VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT}
       */
      void map();

      /**
       \brief Unmap the memory of the Image
       \warning {The image must have been allocated with the memory flag bits set to VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT}
       */
			void unmap();

      /**
       \brief Change the layout of the image
       \param cmdBuff : the command buffer used for this operation, must be in a recording state
       \param newLayout the new layout of the image
       \param dstStage the new stage flag
       \param subresourceRange the range of the memory that will be affected by the operation
       */
			void setLayout(CommandBuffer& cmdbuff,
                     VkImageLayout newLayout,
                     VkPipelineStageFlags dstStage,
                     VkImageSubresourceRange subresourceRange);

      /**
       \brief Copy the content of the image to another image
       \param cmdBuff : the command buffer used for this operation, must be in a recording state
       \param image : the destination image
       \param regions : The list of regions of the image to be copied to the image
       */
			void copyToImage(CommandBuffer& cmdBuff, Image& image, std::vector<VkImageCopy> regions);
      
      /**
       \brief Copy the content of the image to a buffer
       \param cmdBuff : the command buffer used for this operation, must be in a recording state
       \param buffer : the destination buffer
       \param regions : The list of regions of the image to be copied to the buffer
       */
			void copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, std::vector<VkBufferImageCopy> regions);

      /**
       \brief Get the handle of the image
       \return VkImage : the handle of the image
       */
			VkImage& getHandle();
      
      /**
       \brief Get the handle of the image memory
       \return VkDeviceMemory : the image memory of the image
       */
			VkDeviceMemory& getImageMemory();

      /**
       \brief Get the handle of the image view
       \return VkImageView : the image view of the image
       */
			VkImageView& getImageView();

			~Image() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();


				if (VK_NULL_HANDLE != m_image) {
					vkDestroyImage(logical, m_image, nullptr);
					m_image = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_imageView) {
					vkDestroyImageView(logical, m_imageView, nullptr);
					m_imageView = VK_NULL_HANDLE;
				}

				if (VK_NULL_HANDLE != m_imageMemory) {
					vkFreeMemory(logical, m_imageMemory, nullptr);
					m_imageMemory = VK_NULL_HANDLE;
				}

			}

      /**
       \brief Get the Layout of the image
       \return the VkImageLayout of the image
       */
			VkImageLayout& getLayout();

      /**
       \brief Get the witdh of the image
       \return uint32_t the width of the image
       */
			uint32_t width();

      /**
       \brief Get the height of the image
       \return uint32_t the height of the image
       */
			uint32_t height();

      /**
       \brief Get the depth of the image
       \return uint32_t the depth of the image
       */
			uint32_t depth();

    private:

      uint32_t														m_width = 0;
      uint32_t														m_height = 0;
      uint32_t														m_depth = 0;
      VkFormat														m_format;

			VkImageLayout												m_layout;
			VkPipelineStageFlags								m_stage;
			VkImageAspectFlagBits								m_aspect;

      VkImage                             m_image  = VK_NULL_HANDLE;
      VkDeviceMemory                      m_imageMemory  = VK_NULL_HANDLE;
      VkImageView                         m_imageView  = VK_NULL_HANDLE;

			bool																m_cubemap;

			void*																m_mappedMemory;
    };

  }
}

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
       \param usage the usage of the image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkImageUsageFlags.html">here</a>
       \param memPropertyFlag : the memory property of the image, see more <a href="https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkMemoryPropertyFlagBits.html">here</a>
       \param cubemap [otpional] weither or not the image is a cubemap, false by defalt
       */
			Image(
        uint32_t width, 
        uint32_t height, 
        uint32_t depth, 
        VkFormat format, 
        VkImageAspectFlagBits aspect, 
        VkImageUsageFlags usage,
        VkMemoryPropertyFlagBits memPropertyFlag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        bool cubemap = false);


      Image(const Image&) = delete;

      Image& operator=(const Image&) = delete;

      Image(Image&& i) noexcept{

        m_width = i.m_width;
        m_height = i.m_height;
        m_depth = i.m_depth;
        m_format = i.m_format;

        m_layout = i.m_layout;
        m_stage = i.m_stage;
        m_aspect = i.m_aspect;

        m_image = i.m_image;
        m_imageMemory = i.m_imageMemory;
        m_imageView = i.m_imageView;
        m_sampler = i.m_sampler;
        m_cubemap = i.m_cubemap;
        m_mappedMemory = i.m_mappedMemory;

        i.m_image = VK_NULL_HANDLE;
        i.m_imageMemory = VK_NULL_HANDLE;
        i.m_imageView = VK_NULL_HANDLE;
        i.m_sampler = VK_NULL_HANDLE;
        i.m_mappedMemory = nullptr;

      };


      void createSampler();

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
			const VkImage& getHandle() const ;
      
      /**
       \brief Get the handle of the image memory
       \return VkDeviceMemory : the image memory of the image
       */
      const VkDeviceMemory& getImageMemory() const;

      /**
       \brief Get the handle of the image view
       \return VkImageView : the image view of the image
       */
      const VkImageView& getImageView() const;

			~Image() {
				Device* d = Device::getDevice();
				VkDevice logical = d->getLogicalDevice();

        if (VK_NULL_HANDLE != m_sampler) {
          vkDestroySampler(logical, m_sampler, nullptr);
          m_sampler = VK_NULL_HANDLE;
        }

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
      VkImageLayout getLayout() const;


      const VkSampler& getSampler() const;

      /**
       \brief Get the witdh of the image
       \return uint32_t the width of the image
       */
      uint32_t width() const;

      /**
       \brief Get the height of the image
       \return uint32_t the height of the image
       */
       uint32_t height() const;

      /**
       \brief Get the depth of the image
       \return uint32_t the depth of the image
       */
      uint32_t depth() const;

    private:

      uint32_t														m_width = 0;
      uint32_t														m_height = 0;
      uint32_t														m_depth = 0;
      VkFormat														m_format;

			VkImageLayout												m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkPipelineStageFlags								m_stage = VK_PIPELINE_STAGE_NONE_KHR;
			VkImageAspectFlagBits								m_aspect;

      VkImage                             m_image  = VK_NULL_HANDLE;
      VkDeviceMemory                      m_imageMemory  = VK_NULL_HANDLE;
      VkImageView                         m_imageView  = VK_NULL_HANDLE;
      VkSampler             							m_sampler = VK_NULL_HANDLE;

			bool																m_cubemap = false;

			void*																m_mappedMemory = nullptr;
    };

  }
}

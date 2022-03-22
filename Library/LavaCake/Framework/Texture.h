#pragma once

#include "AllHeaders.h"
#include "Queue.h"
#include "Device.h"
#include "ErrorCheck.h"
#include "SwapChain.h"
#include "CommandBuffer.h"
#include "Image.h"
#include <LavaCake/Math/basics.h>"

namespace LavaCake {
  namespace Framework {

    /**
      Class FrameBuffer
      \brief a class to help manage frame buffers
     */
    class FrameBuffer {
    public:

      /**
        \brief create a FrameBuffer.
        \param width: the width of the FrameBuffer.
        \param height: the height of the FrameBuffer.
      */
      FrameBuffer(uint32_t width, uint32_t height);

      /**
        \brief get the image layout of one layer of the FrameBuffer
        \param i: the index of the layer
        \return a VkImageLayout
      */
      VkImageLayout getLayout(uint8_t i) const;


      /**
        \brief get the sampler of the frameBuffer
        \return a VkSampler
      */
      const VkSampler& getSampler() const;

      /**
        \brief get the image view of one layer of the Framebuffer
        \param i the index of the layer
        \brief a VkImageView
      */
      const VkImageView& getImageView(uint8_t i) const;

      /**
        \brief get the number of image view in the Framebuffer
        \return a size_t
      */
      size_t getImageViewSize() const;

      /**
        \brief get the handle of the Framebuffer
        \return a VkFramebuffer
      */
      const VkFramebuffer& getHandle() const;

      /**
        \brief get the resolution of the Framebuffer
        \return a vec2u
      */
      vec2u size() const {
        return vec2u({ m_width , m_height });
      }

      ~FrameBuffer() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        for (uint32_t i = 0; i < m_images.size(); i++) {
          if (i != m_swapChainImageIndex) {
            if (m_images[i] != nullptr) {
              m_images[i] = nullptr;
            }
          }
        }

        if (VK_NULL_HANDLE != m_frameBuffer) {
          vkDestroyFramebuffer(logical, m_frameBuffer, nullptr);
          m_frameBuffer = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != m_sampler) {
          vkDestroySampler(logical, m_sampler, nullptr);
          m_sampler = VK_NULL_HANDLE;
        }

        if (VK_NULL_HANDLE != m_imageMemory) {
          vkFreeMemory(logical, m_imageMemory, nullptr);
          m_imageMemory = VK_NULL_HANDLE;
        }

      }

    private:

      uint32_t																								m_width = 0;
      uint32_t																								m_height = 0;

      VkFramebuffer															              m_frameBuffer = VK_NULL_HANDLE;
      VkSampler																              	m_sampler = VK_NULL_HANDLE;
      VkDeviceMemory            															m_imageMemory = VK_NULL_HANDLE;


      std::vector<std::shared_ptr<Image>>                     m_images;

      uint32_t																								m_swapChainImageIndex = 0;
      friend class RenderPass;
    };


    /**
      Enum attachmentType
      \brief describe the type of attachement
    */
    enum attachmentType {
      COLOR_ATTACHMENT = 1, DEPTH_ATTACHMENT = 2, STENCIL_ATTACHMENT = 4
    };


    /**
      \brief create an image specialized to be an input attachement.
      \param queue: a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff: the command buffer used for this operation, must not be in a recording state
      \param width: the with of the image
      \param height: the height of the image
      \param format: the format of the image
      \param type: the attachment type (see attachementType)
    */
    Image createAttachment(const Queue& queue, CommandBuffer& cmdBuff, int width, int height, VkFormat format, attachmentType type);


    /**
      \brief create an image specialized to be a storage image.
      \param queue: a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff: the command buffer used for this operation, must not be in a recording state
      \param width: the with of the image
      \param height: the height of the image
      \param depth: the depth of the image
      \param format: the format of the image
    */
    Image createStorageImage(const Queue& queue, CommandBuffer& cmdBuff, int width, int height, int depth = 1, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM);

    /**
      \brief create an image specialized to be a texture buffer.
      \param width: the with of the image
      \param height: the height of the image
      \param depth: the depth of the image
      \param format: the format of the image
    */
    Image createTextureBuffer(int width, int height, int depth, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);


    /**
      \brief create an image specialized to be a texture buffer and initialize it with the data contained in a file.
      \param queue: a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff: the command buffer used for this operation, must not be in a recording state
      \param filename: the path to the file containing the data
      \param nbChannel: the number of channel to use from the image
      \param format: the format of the image
      \param stageFlagBit: the stage in which the shader will be used
    */
    Image createTextureBuffer(const Queue& queue, CommandBuffer& cmdBuff, const std::string& filename, int nbChannel, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


    /**
      \brief create an image specialized to be a texture buffer and initialize it.
      \param queue: a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff: the command buffer used for this operation, must not be in a recording state
      \param data: the data used to initialized the file
      \param width: the with of the image
      \param height: the height of the image
      \param depth: the depth of the image
      \param nbChannel: the number of channel in the image
      \param format: the format of the image
      \param stageFlagBit: the stage in which the shader will be used
    */
    Image createTextureBuffer(const Queue& queue, CommandBuffer& cmdBuff, const std::vector<unsigned char>& data, int width, int height, int depth, int nbChannel, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


    /**
      \brief create an image specialized to be a cubemap and initialize it with a set of texture.
      \param queue: a pointer to the queue that will be used to copy data to the Buffer
      \param cmdBuff: the command buffer used for this operation, must not be in a recording state
      \param filename: the path to the folder containing the textures
      \param images: the names of the textures
      \param nbChannel: the number of channel to use from the textures
      \param format: the format of the image
      \param stageFlagBit: the stage in which the shader will be used
    */
    Image createCubeMap(const Queue& queue, CommandBuffer& cmdBuff, const std::string& path, int nbChannel, const std::array<std::string, 6>& images = { "posx.jpg","negx.jpg","posy.jpg","negy.jpg","posz.jpg","negz.jpg" }, VkFormat f = VK_FORMAT_R8G8B8A8_UNORM, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

  }
}

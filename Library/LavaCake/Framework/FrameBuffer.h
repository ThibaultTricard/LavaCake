#pragma once
#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Queue.h"
#include "Device.h"
#include "ErrorCheck.h"
#include "SwapChain.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Image.h"

namespace LavaCake {
  namespace Framework {
  class FrameBuffer {
    public :
    
    /**
     * construct a FrameBuffer.
     *
     * @param int width            : the width of the FrameBuffer.
     * @param int height            : the height of the FrameBuffer.
     * @param int layer            : the number of layer in the FrameBuffer.
     * @param VkFormat f            : the format of theTexture buffer.
     *  @param frameBufferType type : the type of the frame buffer will store : COLOR_FRAMEBUFFER, DEPTH_FRAMEBUFFER or STENCIL_FRAMEBUFFER.
     *
     */
    FrameBuffer(uint32_t width, uint32_t height);
    
    
    
    /**
     * return the layout the texture buffer
     *
     */
    VkImageLayout getLayout(int i);
    
    
    /**
     * return the sampler of the texture buffer
     *
     */
    VkSampler  getSampler();
    
    /**
     * return an image view of the texture buffer
     *
     */
    VkImageView   getImageViews(int i);
    
    /**
     * return number of image view of the texture buffer
     *
     */
    size_t getImageViewSize();
    
    /**
     * return the FrameBuffer handle
     *
     */
    VkFramebuffer getHandle();
    
    vec2u size() {
      return vec2u({ m_width , m_height });
    }
    
    ~FrameBuffer() {
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      
      
      for (uint32_t i = 0; i < m_images.size(); i++) {
        if (i != m_swapChainImageIndex) {
          if (VK_NULL_HANDLE != m_images[i]) {
            vkDestroyImage(logical, m_images[i], nullptr);
            m_images[i] = VK_NULL_HANDLE;
          }
          
          if (VK_NULL_HANDLE != m_imageViews[i]) {
            vkDestroyImageView(logical, m_imageViews[i], nullptr);
            m_imageViews[i] = VK_NULL_HANDLE;
          }
        }
      }
      
      if (VK_NULL_HANDLE != *m_sampler) {
        vkDestroySampler(logical, *m_sampler, nullptr);
        *m_sampler = VK_NULL_HANDLE;
      }
      
      if (VK_NULL_HANDLE != *m_frameBuffer) {
        vkDestroyFramebuffer(logical, *m_frameBuffer, nullptr);
        *m_frameBuffer = VK_NULL_HANDLE;
      }
      
      if (VK_NULL_HANDLE != *m_imageMemory) {
        vkFreeMemory(logical, *m_imageMemory, nullptr);
        *m_imageMemory = VK_NULL_HANDLE;
      }
    }
    
    void copyToImage(CommandBuffer& cmdBuff,uint32_t imageSrc, Image& imageDst, std::vector<VkImageCopy> regions) ;

    void copyToBuffer(CommandBuffer& cmdBuff,uint32_t imageSrc, Buffer& bufferDst, std::vector<VkBufferImageCopy> regions);
    
    void setLayout(CommandBuffer& cmdbuff, uint32_t image, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags dstStage);
    
    template <typename t>
    void readBack(Queue* queue, CommandBuffer& cmdBuff, uint32_t image, std::vector<t>& readBackData){
      Device* d = Device::getDevice();
      VkPhysicalDevice physical = d->getPhysicalDevice();
      VkDevice logical = d->getLogicalDevice();

      Buffer stagingBuffer;
      
      uint32_t dataSize =m_width*m_height*m_pixelByteSizes[image];
      
      stagingBuffer.allocate(dataSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

      readBackData = std::vector<t>(dataSize/sizeof(t));

      cmdBuff.beginRecord();

      VkPipelineStageFlags                                stage  = m_stages[image];
      VkImageLayout                                       layout = m_layouts[image];

      VkImageSubresourceRange subresourceRange{ m_aspects[image], 0, 1, 0, 1 };
      
      setLayout(cmdBuff, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange, VK_PIPELINE_STAGE_TRANSFER_BIT);
      
      copyToBuffer(cmdBuff, image, stagingBuffer, { { 0, 0, dataSize } });

      
      setLayout(cmdBuff, image, layout, subresourceRange, stage);

      cmdBuff.endRecord();

      cmdBuff.submit(queue, {}, {});

      cmdBuff.wait(UINT32_MAX);
      cmdBuff.resetFence();
      void* local_pointer = stagingBuffer.map();

      std::memcpy(&readBackData[0], local_pointer, static_cast<size_t>(dataSize));

      stagingBuffer.unmap();
    }
    
    private :
    
    uint32_t                                                m_width = 0;
    uint32_t                                                m_height = 0;
    
    std::vector<uint32_t>                                   m_pixelByteSizes;
    std::vector<VkPipelineStageFlags>                       m_stages;
    std::vector<VkImageAspectFlagBits>                      m_aspects;
    
    VkDestroyer(VkFramebuffer)                              m_frameBuffer;
    VkDestroyer(VkSampler)                                  m_sampler;
    VkDestroyer(VkDeviceMemory)                             m_imageMemory;
    std::vector<VkImage>                                    m_images;
    std::vector<VkImageView>                                m_imageViews;
    std::vector<VkImageLayout>                              m_layouts;
    
    uint32_t                                                m_swapChainImageIndex;
    friend class RenderPass;
  };
  }
}

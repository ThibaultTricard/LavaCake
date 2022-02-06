#include "FrameBuffer.h"

namespace LavaCake {
  namespace Framework {
  
    FrameBuffer::FrameBuffer(uint32_t width, uint32_t height) {
      m_width = width;
      m_height = height;
    };
  
    VkImageView FrameBuffer::getImageViews(int i) {
      return m_imageViews[i];
    }
  
    size_t FrameBuffer::getImageViewSize(){
      return m_imageViews.size();
    }
  
    VkImageLayout FrameBuffer::getLayout(int i){
      return m_layouts[i];
    }
  
    VkSampler  FrameBuffer::getSampler() {
      return *m_sampler;
    }
  
    VkFramebuffer FrameBuffer::getHandle() {
      return *m_frameBuffer;
    }
  
    void FrameBuffer::setLayout(CommandBuffer& cmdbuff, uint32_t image, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags dstStage) {

      // Create an image barrier object
      VkImageMemoryBarrier imageMemoryBarrier{};
      imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imageMemoryBarrier.oldLayout = m_layouts[image];
      imageMemoryBarrier.newLayout = newLayout;
      imageMemoryBarrier.image = m_images[image];
      imageMemoryBarrier.subresourceRange = subresourceRange;
      
      // Source layouts (old)
      // Source access mask controls actions that have to be finished on the old layout
      // before it will be transitioned to the new layout
      switch (m_layouts[image])
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
                           m_stages[image],
                           dstStage,
                           0,
                           0, nullptr,
                           0, nullptr,
                           1, &imageMemoryBarrier);
      
      m_layouts[image] = newLayout;
      m_stages[image] = dstStage;

    }
  
    void FrameBuffer::copyToImage(CommandBuffer& cmdBuff,uint32_t imageSrc, Image& imageDst, std::vector<VkImageCopy> regions) {
      if (regions.size() > 0) {
        vkCmdCopyImage(cmdBuff.getHandle(), m_images[imageSrc], m_layouts[imageSrc], imageDst.getImage(), imageDst.getLayout(), static_cast<uint32_t>(regions.size()), regions.data());
      }
    }

    void FrameBuffer::copyToBuffer(CommandBuffer& cmdBuff, uint32_t imageSrc, Buffer& bufferDst, std::vector<VkBufferImageCopy> regions) {
      if (regions.size() > 0) {
        vkCmdCopyImageToBuffer(cmdBuff.getHandle(), m_images[imageSrc], m_layouts[imageSrc], bufferDst.getHandle(), static_cast<uint32_t>(regions.size()), regions.data());
      }
    }
  }
}

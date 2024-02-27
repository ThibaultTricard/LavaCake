#pragma once

#include "AllHeaders.h"
#include <vector>


namespace LavaCake {
  namespace Framework {

  class Attachment{
  public:
    
    Attachment(){};
    
    VkImageUsageFlagBits m_usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
    VkImageAspectFlagBits m_aspect = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkAttachmentDescription m_attachmentDescription;
  };


  class ColorAttachment : public Attachment{
    public :
    
    ColorAttachment(VkFormat imageFormat): Attachment(){
      m_attachmentDescription = {
        0,                                        // VkAttachmentDescriptionFlags     flags
        imageFormat,                              // VkFormat                         format
        VK_SAMPLE_COUNT_1_BIT,                    // VkSampleCountFlagBits            samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,              // VkAttachmentLoadOp               loadOp
        VK_ATTACHMENT_STORE_OP_STORE,             // VkAttachmentStoreOp              storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,          // VkAttachmentLoadOp               stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,         // VkAttachmentStoreOp              stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout                    initialLayout
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL  // VkImageLayout                    finalLayout
      };
      
      m_format = imageFormat;
      m_usage = VkImageUsageFlagBits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
      m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    VkPipelineColorBlendAttachmentState m_blendState = {
      VK_FALSE,                                       // VkBool32                 blendEnable
      VK_BLEND_FACTOR_SRC_ALPHA,                      // VkBlendFactor            srcColorBlendFactor
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,            // VkBlendFactor            dstColorBlendFactor
      VK_BLEND_OP_ADD,                                // VkBlendOp                colorBlendOp
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,            // VkBlendFactor            srcAlphaBlendFactor
      VK_BLEND_FACTOR_ZERO,                           // VkBlendFactor            dstAlphaBlendFactor
      VK_BLEND_OP_ADD,                                // VkBlendOp                alphaBlendOp
      VK_COLOR_COMPONENT_R_BIT |                      // VkColorComponentFlags    colorWriteMask
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT
    };
    
    
  };

  
  class DepthAttachment : public Attachment{
  public:
    DepthAttachment(VkFormat depthFormat):Attachment(){
      m_attachmentDescription = {
        0,                                                  // VkAttachmentDescriptionFlags     flags
        depthFormat,                                        // VkFormat                         format
        VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits            samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,                        // VkAttachmentLoadOp               loadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp              storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,                    // VkAttachmentLoadOp               stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,                   // VkAttachmentStoreOp              stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,                          // VkImageLayout                    initialLayout
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL     // VkImageLayout                    finalLayout
      };
      
      m_format = depthFormat;
      m_usage = VkImageUsageFlagBits(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      
      if (
        m_format == VK_FORMAT_D16_UNORM ||
        m_format == VK_FORMAT_X8_D24_UNORM_PACK32 ||
        m_format == VK_FORMAT_D32_SFLOAT) {
        m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
      }
      else if (m_format == VK_FORMAT_S8_UINT) {
        m_aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
      }
      else if (
        m_format == VK_FORMAT_D16_UNORM_S8_UINT ||
        m_format == VK_FORMAT_D24_UNORM_S8_UINT ||
        m_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        m_aspect = VkImageAspectFlagBits(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
      }
      
      m_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
    
    VkPipelineDepthStencilStateCreateInfo m_depthStencilStateCreateInfo= {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,   // VkStructureType                            sType
      nullptr,                                                      // const void                               * pNext
      0,                                                            // VkPipelineDepthStencilStateCreateFlags     flags
      true,                                                         // VkBool32                              depthTestEnable
      true,                                                         // VkBool32                             depthWriteEnable
      VK_COMPARE_OP_LESS_OR_EQUAL,                                  // VkCompareOp                            depthCompareOp
      false,                                                        // VkBool32                        depthBoundsTestEnable
      false,                                                        // VkBool32                            stencilTestEnable
      {},                                                           // VkStencilOpState                           front
      {},                                                           // VkStencilOpState                           back
      0.0f,                                                         // float                                  minDepthBounds
      1.0f                                                          // float                                  maxDepthBounds
    };
  };

  class SwapChainImageAttachment : public ColorAttachment{
  public:
    
    SwapChainImageAttachment(VkFormat imageFormat) : ColorAttachment(imageFormat){
      m_attachmentDescription = {
        0,                                      // VkAttachmentDescriptionFlags     flags
        imageFormat,                            // VkFormat                         format
        VK_SAMPLE_COUNT_1_BIT,                  // VkSampleCountFlagBits            samples
        VK_ATTACHMENT_LOAD_OP_CLEAR,            // VkAttachmentLoadOp               loadOp
        VK_ATTACHMENT_STORE_OP_STORE,           // VkAttachmentStoreOp              storeOp
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,        // VkAttachmentLoadOp               stencilLoadOp
        VK_ATTACHMENT_STORE_OP_DONT_CARE,       // VkAttachmentStoreOp              stencilStoreOp
        VK_IMAGE_LAYOUT_UNDEFINED,              // VkImageLayout                    initialLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR         // VkImageLayout                    finalLayout
      };
      
      m_attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      
      m_format = imageFormat;
      m_usage = VkImageUsageFlagBits(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
      m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;
      m_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    
    
    
  };


  class SubPassAttachments{
    public :
    SubPassAttachments(VkFormat depthFormat) : m_depthAttachments(depthFormat){
      m_useDepth = true;
    }
    
    SubPassAttachments() : m_depthAttachments(VK_FORMAT_UNDEFINED){}
    
    void setDepthFormat(VkFormat depthFormat){
      m_depthAttachments = DepthAttachment(depthFormat);
      m_useDepth = true;
    }
    
    int addColorAttachment(VkFormat imageFormat){
      m_attachments.emplace_back(ColorAttachment(imageFormat));
      m_nbColorAttachment++;
      return static_cast<int>(m_attachments.size()) - 1;
    }
    
    int addSwapChainImageAttachment(VkFormat imageFormat){
      m_attachments.emplace_back(SwapChainImageAttachment(imageFormat));
      m_nbColorAttachment++;
      m_hasASwapChainImage = true;
      m_swapChainImageIndex = static_cast<uint32_t>(m_attachments.size()) - 1;
      return static_cast<int>(m_attachments.size()) - 1;
    }
    
    uint32_t m_nbColorAttachment = 0;
    bool m_hasASwapChainImage = false;
    uint32_t m_swapChainImageIndex = 0;
    std::vector<ColorAttachment> m_attachments;
    DepthAttachment m_depthAttachments;
    bool m_useDepth = false;
  };

  }
}

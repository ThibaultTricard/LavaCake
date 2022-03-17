#include "Image.h"

namespace LavaCake {
  namespace Framework {


    Image::Image(uint32_t width, uint32_t height, uint32_t depth, VkFormat f, VkImageAspectFlagBits aspect, VkImageUsageFlags usage,
      VkMemoryPropertyFlagBits memPropertyFlag, bool cubemap) {
      m_width = width;
      m_height = height;
      m_depth = depth;
      m_format = f;
      m_aspect = aspect;
      m_cubemap = cubemap;

      Framework::Device* d = LavaCake::Framework::Device::getDevice();
      VkDevice logical = d->getLogicalDevice();

      VkPhysicalDevice physical = d->getPhysicalDevice();

      VkImageType type = VK_IMAGE_TYPE_1D;
      VkImageViewType view = VK_IMAGE_VIEW_TYPE_1D;
      if (m_height > 1) { type = VK_IMAGE_TYPE_2D; view = VK_IMAGE_VIEW_TYPE_2D; }
      if (m_depth > 1) { type = VK_IMAGE_TYPE_3D; view = VK_IMAGE_VIEW_TYPE_3D; }
      if (m_cubemap) { type = VK_IMAGE_TYPE_2D; view = VK_IMAGE_VIEW_TYPE_CUBE; }

      // image creation
      VkImageCreateInfo image_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                // VkStructureType          sType
        nullptr,                                            // const void             * pNext
        cubemap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u, // VkImageCreateFlags       flags
        type,                                               // VkImageType              imageType
        m_format,                                           // VkFormat                 format
        { m_width, m_height, m_depth },                     // VkExtent3D               extent
        1,																									// uint32_t                 mipLevels
        cubemap ? (uint32_t)6 : (uint32_t)1,								// uint32_t                 arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                              // VkSampleCountFlagBits    samples
        VK_IMAGE_TILING_OPTIMAL,                            // VkImageTiling            tiling
        usage,																							// VkImageUsageFlags        usage
        VK_SHARING_MODE_EXCLUSIVE,                          // VkSharingMode            sharingMode
        0,                                                  // uint32_t                 queueFamilyIndexCount
        nullptr,                                            // const uint32_t         * pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED                           // VkImageLayout            initialLayout
      };

      VkResult result = vkCreateImage(logical, &image_create_info, nullptr, &m_image);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Could not create an image.");
      }


      // memory allocation
      VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
      vkGetPhysicalDeviceMemoryProperties(physical, &physical_device_memory_properties);

      VkMemoryRequirements memory_requirements;
      vkGetImageMemoryRequirements(logical, m_image, &memory_requirements);

      m_imageMemory = VK_NULL_HANDLE;
      for (uint32_t type = 0; type < physical_device_memory_properties.memoryTypeCount; ++type) {
        if ((memory_requirements.memoryTypeBits & (1 << type)) &&
          ((physical_device_memory_properties.memoryTypes[type].propertyFlags & memPropertyFlag) == memPropertyFlag)) {

          VkMemoryAllocateInfo image_memory_allocate_info = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,   // VkStructureType    sType
            nullptr,                                  // const void       * pNext
            memory_requirements.size,                 // VkDeviceSize       allocationSize
            type                                      // uint32_t           memoryTypeIndex
          };

          VkResult result = vkAllocateMemory(logical, &image_memory_allocate_info, nullptr, &m_imageMemory);
          if (VK_SUCCESS == result) {
            break;
          }
        }
      }

      if (m_imageMemory == VK_NULL_HANDLE) {
        ErrorCheck::setError("Could not allocate memory for an image.");
      }


      //memory binding
      result = vkBindImageMemory(logical, m_image, m_imageMemory, 0);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Could not bind memory object to an image.");
      }


      // image view creation
      VkImageViewCreateInfo image_view_create_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,			// VkStructureType            sType
      nullptr,																			// const void               * pNext
      0,																						// VkImageViewCreateFlags     flags
      m_image,                                      // VkImage                    image
      view,																					// VkImageViewType            viewType
      m_format,                                     // VkFormat                   format
      {																							// VkComponentMapping         components
        VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         r
        VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         g
        VK_COMPONENT_SWIZZLE_IDENTITY,              // VkComponentSwizzle         b
        VK_COMPONENT_SWIZZLE_IDENTITY               // VkComponentSwizzle         a
      },
      {																							// VkImageSubresourceRange    subresourceRange
        (VkImageAspectFlags)m_aspect,								// VkImageAspectFlags         aspectMask
        0,                                          // uint32_t                   baseMipLevel
        VK_REMAINING_MIP_LEVELS,                    // uint32_t                   levelCount
        0,                                          // uint32_t                   baseArrayLayer
        VK_REMAINING_ARRAY_LAYERS                   // uint32_t                   layerCount
      }
      };

      result = vkCreateImageView(logical, &image_view_create_info, nullptr, &m_imageView);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Can't create Image View");
      }


      m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
      m_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    }


    void Image::createSampler() {
      auto device = Device::getDevice();
      auto logical = device->getLogicalDevice();


      VkSamplerCreateInfo sampler_create_info = {
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,    // VkStructureType          sType
        nullptr,                                  // const void             * pNext
        0,                                        // VkSamplerCreateFlags     flags
        VK_FILTER_LINEAR,                         // VkFilter                 magFilter
        VK_FILTER_LINEAR,                         // VkFilter                 minFilter
        VK_SAMPLER_MIPMAP_MODE_NEAREST,           // VkSamplerMipmapMode      mipmapMode
        VK_SAMPLER_ADDRESS_MODE_REPEAT,           // VkSamplerAddressMode     addressModeU
        VK_SAMPLER_ADDRESS_MODE_REPEAT,           // VkSamplerAddressMode     addressModeV
        VK_SAMPLER_ADDRESS_MODE_REPEAT,           // VkSamplerAddressMode     addressModeW
        0.0f,																			// float                    mipLodBias
        false,																		// VkBool32                 anisotropyEnable
        1.0f,																			// float                    maxAnisotropy
        false,																		// VkBool32                 compareEnable
        VK_COMPARE_OP_ALWAYS,                     // VkCompareOp              compareOp
        0.0f,																			// float                    minLod
        1.0f,																			// float                    maxLod
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,       // VkBorderColor            borderColor
        false																			// VkBool32                 unnormalizedCoordinates
      };

      VkResult result = vkCreateSampler(logical, &sampler_create_info, nullptr, &m_sampler);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Could not create sampler.");;
      }
    }

    void Image::map() {
      Framework::Device* d = LavaCake::Framework::Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      vkMapMemory(logical, m_imageMemory, 0, VK_WHOLE_SIZE, 0, &m_mappedMemory);
    }

    void Image::unmap() {
      Framework::Device* d = LavaCake::Framework::Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      vkUnmapMemory(logical, m_imageMemory);
    }


    void Image::setLayout(CommandBuffer& cmdbuff, VkImageLayout newLayout, VkPipelineStageFlags dstStage, VkImageSubresourceRange subresourceRange) {

      // Create an image barrier object
      VkImageMemoryBarrier imageMemoryBarrier{};
      imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imageMemoryBarrier.oldLayout = m_layout;
      imageMemoryBarrier.newLayout = newLayout;
      imageMemoryBarrier.image = m_image;
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
          //imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
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

    void Image::copyToImage(CommandBuffer& cmdBuff, Image& image, std::vector<VkImageCopy> regions) {
      if (regions.size() > 0) {
        vkCmdCopyImage(cmdBuff.getHandle(), m_image, m_layout, image.getHandle(), image.getLayout(), static_cast<uint32_t>(regions.size()), regions.data());
      }
    }

    void Image::copyToBuffer(CommandBuffer& cmdBuff, Buffer& buffer, std::vector<VkBufferImageCopy> regions) {
      if (regions.size() > 0) {
        vkCmdCopyImageToBuffer(cmdBuff.getHandle(), m_image, m_layout, buffer.getHandle(), static_cast<uint32_t>(regions.size()), regions.data());
      }
    }

    const VkImage& Image::getHandle() const {
      return m_image;
    }

    const VkDeviceMemory& Image::getImageMemory() const {
      return m_imageMemory;
    }

    const VkImageView& Image::getImageView() const {
      return m_imageView;
    }

    VkImageLayout Image::getLayout() const {
      return m_layout;
    }

    const VkSampler& Image::getSampler() const {
      return m_sampler;
    }

    uint32_t Image::width() const {
      return m_width;
    }

    uint32_t Image::height() const {
      return m_height;
    }

    uint32_t Image::depth() const {
      return m_depth;
    }


  }
}

#include "DescriptorSet.h"

namespace LavaCake {
  namespace Framework {

    VkDescriptorSetLayout generateEmptyLayout(){
      Device* d = Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      
      VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,                    // VkStructureType                      sType
        nullptr,                                                                // const void                         * pNext
        0,                                                                      // VkDescriptorSetLayoutCreateFlags     flags
        static_cast<uint32_t>(0),                                               // uint32_t                             bindingCount
        {}                                                                      // const VkDescriptorSetLayoutBinding * pBindings
      };
      VkDescriptorSetLayout layout;
      VkResult result = vkCreateDescriptorSetLayout(logical, &descriptor_set_layout_create_info, nullptr, &layout);
      if (VK_SUCCESS != result) {
        ErrorCheck::setError("Could not create a layout for descriptor sets.");
      }

      return layout;
    };
  }
}
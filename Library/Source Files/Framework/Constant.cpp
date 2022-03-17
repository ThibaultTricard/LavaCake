#include "Constant.h"

namespace LavaCake {
  namespace Framework {
    void PushConstant::push(VkCommandBuffer buffer, VkPipelineLayout layout, VkShaderStageFlags flag) {
      LavaCake::vkCmdPushConstants(buffer, layout, flag, 0, static_cast<uint32_t>(m_variables.data().size()), m_variables.data().data());
    }

    uint32_t PushConstant::size() {
      return static_cast<uint32_t>(m_variables.data().size());
    }
  }
}

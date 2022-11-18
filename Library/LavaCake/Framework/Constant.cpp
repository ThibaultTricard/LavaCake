#include "Constant.h"


namespace LavaCake {
  namespace Framework {
    void PushConstant::push(const CommandBuffer& buffer, const VkPipelineLayout& layout, const VkPushConstantRange& range) {
      LavaCake::vkCmdPushConstants(buffer.getHandle(), layout, range.stageFlags, range.offset, size(), m_variables.data().data());
    }

    uint32_t PushConstant::size() {
      return static_cast<uint32_t>(m_variables.data().size());
    }
  }
}

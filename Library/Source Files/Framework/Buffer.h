#pragma once

#include "CommandBuffer.h"
#include "Queue.h"

namespace LavaCake {
  namespace Framework {
    
    class Buffer {
    public :

      template<uint32_t usage>
      Buffer();

      void allocate(Queue queue, CommandBuffer cmdBuff,std::vector<float> rawdata, uint32_t dataSize = 1, VkPipelineStageFlagBits stageFlagBit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

      vkBuffer getHandle();

      VkBufferView getBufferView();

      void readBack(Queue queue, CommandBuffer cmdBuff, std::vector<float>& data);

    protected:

      VkDestroyer(VkBuffer)																m_buffer;
      VkDestroyer(VkDeviceMemory)													m_bufferMemory;
      VkDestroyer(VkBufferView)														m_bufferView;
    };


    template <uint32_t topology>
    class VBuffer public Buffer {

    };



    using TriangleVertexBuffer = VBuffer<VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST>;
    using LineVertexBuffer = VBuffer<VK_PRIMITIVE_TOPOLOGY_LINE_LIST>;
  }
}
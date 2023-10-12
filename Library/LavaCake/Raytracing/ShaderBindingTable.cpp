#include <LavaCake/Raytracing/ShaderBindingTable.h>

namespace LavaCake {
  namespace RayTracing {

    void ShaderBindingTable::compile(const Framework::Queue& queue, Framework::CommandBuffer& cmdBuff, VkPipeline raytracingPipeline) {
      Framework::Device* d = Framework::Device::getDevice();
      VkDevice logical = d->getLogicalDevice();
      VkPhysicalDevice physical = d->getPhysicalDevice();

      VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};

      rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
      VkPhysicalDeviceProperties2 deviceProperties2{};
      deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
      deviceProperties2.pNext = &rayTracingPipelineProperties;
      vkGetPhysicalDeviceProperties2(physical, &deviceProperties2);

      const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
      const uint32_t handleSizeAligned = alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
      const uint32_t groupCount = static_cast<uint32_t>(m_rayGen.size() + m_miss.size() + m_hitGroup.size());
      const uint32_t sbtSize = groupCount * handleSizeAligned;

      std::vector<uint8_t> shaderHandleStorage(sbtSize);
      vkGetRayTracingShaderGroupHandlesKHR(logical, raytracingPipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());

      // Create buffer to hold all shader handles for the SBT
      
      void* raygenMem;
      m_raygenBuffer = std::make_shared<Framework::Buffer>(handleSize * m_rayGen.size(), VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
      m_raygenShaderBindingTable = {};
      m_raygenShaderBindingTable.deviceAddress = m_raygenBuffer->getBufferDeviceAddress();
      m_raygenShaderBindingTable.stride = handleSizeAligned;
      m_raygenShaderBindingTable.size = m_rayGen.size() * handleSizeAligned;
      vkMapMemory(logical, m_raygenBuffer->getMemory(), 0, VK_WHOLE_SIZE, 0, &raygenMem);

      void* missMem;
      m_missBuffer = std::make_shared<Framework::Buffer>(handleSize * m_miss.size(), VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
      m_missShaderBindingTable = {};
      m_missShaderBindingTable.deviceAddress = m_missBuffer->getBufferDeviceAddress();
      m_missShaderBindingTable.stride = handleSizeAligned;
      m_missShaderBindingTable.size = m_miss.size() * handleSizeAligned;
      vkMapMemory(logical, m_missBuffer->getMemory(), 0, VK_WHOLE_SIZE, 0, &missMem);

      void* hitMem;
      m_hitBuffer = std::make_shared<Framework::Buffer>(handleSize * m_hitGroup.size(), VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
      m_hitShaderBindingTable = {};
      m_hitShaderBindingTable.deviceAddress = m_hitBuffer->getBufferDeviceAddress();
      m_hitShaderBindingTable.stride = handleSizeAligned;
      m_hitShaderBindingTable.size = m_hitGroup.size() * handleSizeAligned;
      vkMapMemory(logical, m_hitBuffer->getMemory(), 0, VK_WHOLE_SIZE, 0, &hitMem);

      // Copy handles
      memcpy(raygenMem, shaderHandleStorage.data(), handleSize * m_rayGen.size());
      memcpy(missMem, shaderHandleStorage.data() + handleSizeAligned * m_rayGen.size(), handleSize * m_miss.size());
      memcpy(hitMem, shaderHandleStorage.data() + handleSizeAligned * (m_rayGen.size() + m_miss.size()), handleSize * m_hitGroup.size());
    }
  }
}
#pragma once
#include "AllHeaders.h"
#include "RayTracing/RayTracingShaderModule.h"
#include "Framework/Buffer.h"



namespace LavaCake {
  namespace RayTracing {


    class ShaderBindingTable {
    public :
      void addRayGeneration(
        uint32_t                          groupIndex,
        const std::vector<unsigned char>& inlineData = {})
      {
        m_rayGen.push_back({ groupIndex, inlineData });
      }

      void addMissProgram(uint32_t                          groupIndex,
        const std::vector<unsigned char>& inlineData)
      {
        m_miss.push_back({ groupIndex, inlineData });
      }

      void addHitGroup(uint32_t                          groupIndex,
        const std::vector<unsigned char >& inlineData)
      {
        m_hitGroup.push_back({ groupIndex, inlineData });
      }


      void compile(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff,VkPipeline raytracingPipeline) {
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
        const uint32_t groupCount = static_cast<uint32_t>(m_rayGen.size() + m_miss.size()  + m_hitGroup.size() );
        const uint32_t sbtSize = groupCount * handleSizeAligned;


        std::vector<uint8_t> shaderHandleStorage(sbtSize);
        vkGetRayTracingShaderGroupHandlesKHR(logical, raytracingPipeline, 0, groupCount, sbtSize, shaderHandleStorage.data());


      
        // Create buffer to hold all shader handles for the SBT
        Framework::Buffer raygenBuffer;
        void* raygenMem;
        raygenBuffer.allocate(queue, cmdBuff, handleSize * m_rayGen.size(), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT), VK_FORMAT_R32_SFLOAT, VK_ACCESS_TRANSFER_WRITE_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        m_raygenShaderBindingTable = {};
        m_raygenShaderBindingTable.deviceAddress = raygenBuffer.getBufferDeviceAddress();
        m_raygenShaderBindingTable.stride = handleSizeAligned;
        m_raygenShaderBindingTable.size = m_rayGen.size() * handleSizeAligned;
        vkMapMemory(logical, raygenBuffer.getMemory(), 0, VK_WHOLE_SIZE, 0, &raygenMem);

        
        Framework::Buffer missBuffer;
        void* missMem;
        missBuffer.allocate(queue, cmdBuff, handleSize * m_miss.size(), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT), VK_FORMAT_R32_SFLOAT, VK_ACCESS_TRANSFER_WRITE_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        m_missShaderBindingTable = {};
        m_missShaderBindingTable.deviceAddress = missBuffer.getBufferDeviceAddress();
        m_missShaderBindingTable.stride = handleSizeAligned;
        m_missShaderBindingTable.size = m_miss.size() * handleSizeAligned;
        vkMapMemory(logical, missBuffer.getMemory(), 0, VK_WHOLE_SIZE, 0, &missMem);


        Framework::Buffer hitBuffer;
        void* hitMem;
        hitBuffer.allocate(queue, cmdBuff, handleSize * m_hitGroup.size(), (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT), VK_FORMAT_R32_SFLOAT, VK_ACCESS_TRANSFER_WRITE_BIT, VkMemoryPropertyFlagBits(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        m_hitShaderBindingTable = {};
        m_hitShaderBindingTable.deviceAddress = hitBuffer.getBufferDeviceAddress();
        m_hitShaderBindingTable.stride = handleSizeAligned;
        m_hitShaderBindingTable.size = m_hitGroup.size() * handleSizeAligned;
        vkMapMemory(logical, hitBuffer.getMemory(), 0, VK_WHOLE_SIZE, 0, &hitMem);

        

        // Copy handles
        memcpy(raygenMem, shaderHandleStorage.data(), handleSize * m_rayGen.size());
        memcpy(missMem, shaderHandleStorage.data() + handleSizeAligned * m_rayGen.size(), handleSize * m_miss.size());
        memcpy(hitMem, shaderHandleStorage.data() + handleSizeAligned * (m_rayGen.size() + m_miss.size()) , handleSize * m_hitGroup.size());
      }


      VkStridedDeviceAddressRegionKHR raygenShaderBindingTable() {
        return m_raygenShaderBindingTable;
      }

      VkStridedDeviceAddressRegionKHR missShaderBindingTable() {
        return m_missShaderBindingTable;
      }

      VkStridedDeviceAddressRegionKHR hitShaderBindingTable() {
        return m_hitShaderBindingTable;
      }
      
      
      VkStridedDeviceAddressRegionKHR callableShaderBindingTable() {
        return m_callableShaderBindingTable;
      }


    private : 
      struct entry
      {
        uint32_t                         m_groupIndex;
        const std::vector<unsigned char> m_inlineData;
      };

     

      uint32_t alignedSize(uint32_t value, uint32_t alignment)
      {
        return (value + alignment - 1) & ~(alignment - 1);
      }

      // Ray generation shader entries
      std::vector<entry> m_rayGen;
      /// Miss shader entries
      std::vector<entry> m_miss;
      /// Hit group entries
      std::vector<entry> m_hitGroup;


      VkStridedDeviceAddressRegionKHR m_raygenShaderBindingTable;
      VkStridedDeviceAddressRegionKHR m_missShaderBindingTable;
      VkStridedDeviceAddressRegionKHR m_hitShaderBindingTable;
      VkStridedDeviceAddressRegionKHR m_callableShaderBindingTable;

      uint32_t m_rayGenEntrySize;
      uint32_t m_missEntrySize;
      uint32_t m_hitGroupEntrySize;

      UINT m_progIdSize;
      VkDeviceSize m_sbtSize;

    };

  }
}
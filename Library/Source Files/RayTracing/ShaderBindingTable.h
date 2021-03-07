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

      void compile(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff, VkPipeline raytracingPipeline);

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
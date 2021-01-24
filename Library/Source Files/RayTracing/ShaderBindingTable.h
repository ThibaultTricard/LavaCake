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


      void compile(Framework::Queue* queue, Framework::CommandBuffer& cmdBuff,VkPipeline raytracingPipeline, uint32_t handleSize) {
        Framework::Device* d = Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();

        Framework::Buffer sbtBuff;
        VkDeviceSize shaderBindingTableSize = ComputeSBTSize(handleSize);

        sbtBuff.allocate(queue, cmdBuff, shaderBindingTableSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_FORMAT_R32_SFLOAT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        uint32_t groupCount = static_cast<uint32_t>(m_rayGen.size())
          + static_cast<uint32_t>(m_miss.size())
          + static_cast<uint32_t>(m_hitGroup.size());

        // Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
        // Note that this could be also done by fetching the handles one by one when writing the SBT entries
        auto     shaderHandleStorage = new uint8_t[groupCount * m_progIdSize];
        VkResult code =
          vkGetRayTracingShaderGroupHandlesNV(logical, raytracingPipeline, 0, groupCount,
            m_progIdSize * groupCount, shaderHandleStorage);

        // Map the SBT
        void* vData;

        code = vkMapMemory(logical, sbtBuff.getMemory(), 0, m_sbtSize, 0, &vData);

        if (code != VK_SUCCESS)
        {
          throw std::logic_error("SBT vkMapMemory failed");
        }

        auto* data = static_cast<uint8_t*>(vData);

        // Copy the shader identifiers followed by their resource pointers or root constants: first the
        // ray generation, then the miss shaders, and finally the set of hit groups
        VkDeviceSize offset = 0;

        offset = CopyShaderData(logical, raytracingPipeline, data, m_rayGen, m_rayGenEntrySize,
          shaderHandleStorage);
        data += offset;

        offset = CopyShaderData(logical, raytracingPipeline, data, m_miss, m_missEntrySize,
          shaderHandleStorage);
        data += offset;

        offset = CopyShaderData(logical, raytracingPipeline, data, m_hitGroup, m_hitGroupEntrySize,
          shaderHandleStorage);

        // Unmap the SBT
        vkUnmapMemory(logical, sbtBuff.getMemory());

        delete shaderHandleStorage;
      }


      
      VkDeviceSize ComputeSBTSize(
        uint32_t handleSize)
      {
        // Size of a program identifier
        m_progIdSize = handleSize;

        // Compute the entry size of each program type depending on the maximum number of parameters in
        // each category
        m_rayGenEntrySize = GetEntrySize(m_rayGen);
        m_missEntrySize = GetEntrySize(m_miss);
        m_hitGroupEntrySize = GetEntrySize(m_hitGroup);

        // The total SBT size is the sum of the entries for ray generation, miss and hit groups
        m_sbtSize = m_rayGenEntrySize * static_cast<VkDeviceSize>(m_rayGen.size())
          + m_missEntrySize * static_cast<VkDeviceSize>(m_miss.size())
          + m_hitGroupEntrySize * static_cast<VkDeviceSize>(m_hitGroup.size());
        return m_sbtSize;
      }

    private : 
      struct entry
      {
        uint32_t                         m_groupIndex;
        const std::vector<unsigned char> m_inlineData;
      };

      VkDeviceSize CopyShaderData(VkDevice                     device,
        VkPipeline                   pipeline,
        uint8_t* outputData,
        const std::vector<entry>& shaders,
        VkDeviceSize                 entrySize,
        const uint8_t* shaderHandleStorage)
      {
        uint8_t* pData = outputData;
        for (const auto& shader : shaders)
        {
          // Copy the shader identifier that was previously obtained with
          // vkGetRayTracingShaderGroupHandlesNV
          memcpy(pData, shaderHandleStorage + shader.m_groupIndex * m_progIdSize, m_progIdSize);

          // Copy all its resources pointers or values in bulk
          if (!shader.m_inlineData.empty())
          {
            memcpy(pData + m_progIdSize, shader.m_inlineData.data(), shader.m_inlineData.size());
          }

          pData += entrySize;
        }
        // Return the number of bytes actually written to the output buffer
        return static_cast<uint32_t>(shaders.size()) * entrySize;
      }

      VkDeviceSize GetEntrySize(const std::vector<entry>& entries)
      {
        // Find the maximum number of parameters used by a single entry
        size_t maxArgs = 0;
        for (const auto& shader : entries)
        {
          maxArgs = maxArgs > shader.m_inlineData.size() ? maxArgs : shader.m_inlineData.size();
        }
        // A SBT entry is made of a program ID and a set of 4-byte parameters (offsets or push constants)
        VkDeviceSize entrySize = m_progIdSize + static_cast<VkDeviceSize>(maxArgs);

        // The entries of the shader binding table must be 16-bytes-aligned
        VkDeviceSize modEntrySize = entrySize % 16;
        if (modEntrySize != 0) {
          entrySize += 16 - modEntrySize;
        }

        return entrySize;
      }

      // Ray generation shader entries
      std::vector<entry> m_rayGen;
      /// Miss shader entries
      std::vector<entry> m_miss;
      /// Hit group entries
      std::vector<entry> m_hitGroup;

      uint32_t m_rayGenEntrySize;
      uint32_t m_missEntrySize;
      uint32_t m_hitGroupEntrySize;

      UINT m_progIdSize;
      VkDeviceSize m_sbtSize;

    };

  }
}
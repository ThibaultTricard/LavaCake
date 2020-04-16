#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"
#include "VulkanDestroyer.h"


namespace LavaCake {
  namespace Framework{
    class UniformBuffer {
    public :
      template<typename T>
      void addVariable(std::string name, T value) {
        std::vector<int> v = std::vector<int>(sizeof(value)  / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      template<typename T>
      void setVariable(std::string name, T value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        setArray(name, v);
      }

      void end() {
        m_modified = std::vector<bool>(m_variables.size());
        LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice physical = d->getPhysicalDevice();

        for (int i = 0; i < m_variables.size(); i++) {
          VkDeviceSize s = sizeof(m_variables[i][0]) * m_variables[i].size();
          m_typeSizeOffset.push_back(std::pair<VkDeviceSize, VkDeviceSize>(s, m_bufferSize));
          m_bufferSize += s;
        }

        InitVkDestroyer(logical, m_stagingBuffer);
        if (!LavaCake::Buffer::CreateBuffer(logical, m_bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, *m_stagingBuffer)) {
          ErrorCheck::setError(8, "Cannot create staging buffer");
        }
        InitVkDestroyer(logical, m_stagingBufferMemory);
        if (!LavaCake::Buffer::AllocateAndBindMemoryObjectToBuffer(physical, logical, *m_stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, *m_stagingBufferMemory)) {
          ErrorCheck::setError(8, "Cannot allocate staging buffer memory");
        }

        InitVkDestroyer(logical, m_buffer);
        InitVkDestroyer(logical, m_bufferMemory);
        if (!Buffer::CreateUniformBuffer(physical, logical, m_bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
          *m_buffer, *m_bufferMemory)) {
          ErrorCheck::setError(8, "Cannot create uniforme buffer");
        }
        copyToStageMemory(true);
      }

      void update(VkCommandBuffer& commandBuffer, bool all = true) {
        copyToStageMemory(all);

        Buffer::BufferTransition pre_transfer_transition = {
          *m_buffer,                      // VkBuffer         Buffer
          VK_ACCESS_UNIFORM_READ_BIT,   // VkAccessFlags    CurrentAccess
          VK_ACCESS_TRANSFER_WRITE_BIT, // VkAccessFlags    NewAccess
          VK_QUEUE_FAMILY_IGNORED,      // uint32_t         CurrentQueueFamily
          VK_QUEUE_FAMILY_IGNORED       // uint32_t         NewQueueFamily
        };
        Buffer::SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, { pre_transfer_transition });

        std::vector<VkBufferCopy> regions = {
          {
            0,                        // VkDeviceSize     srcOffset
            0,                        // VkDeviceSize     dstOffset
            m_bufferSize                // VkDeviceSize     size
          }
        };
        Memory::CopyDataBetweenBuffers(commandBuffer, *m_stagingBuffer, *m_buffer, regions);

        Buffer::BufferTransition post_transfer_transition = {
          *m_buffer,                      // VkBuffer         Buffer
          VK_ACCESS_TRANSFER_WRITE_BIT, // VkAccessFlags    CurrentAccess
          VK_ACCESS_UNIFORM_READ_BIT,   // VkAccessFlags    NewAccess
          VK_QUEUE_FAMILY_IGNORED,      // uint32_t         CurrentQueueFamily
          VK_QUEUE_FAMILY_IGNORED       // uint32_t         NewQueueFamily
        };
        Buffer::SetBufferMemoryBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, { post_transfer_transition });
      }

      VkBuffer& getBuffer() {
        return *m_buffer;
      };

    private :

      void copyToStageMemory(bool all = false) {
        LavaCake::Framework::Device* d = LavaCake::Framework::Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        VkPhysicalDevice physical = d->getPhysicalDevice();
        for (int i = 0; i < m_variables.size(); i++) {
          if (all || m_modified[i]) {
            VkDeviceSize size = m_typeSizeOffset[i].first;
            VkDeviceSize offset = m_typeSizeOffset[i].second;

            if (!Memory::MapUpdateAndUnmapHostVisibleMemory(logical, *m_stagingBufferMemory, offset, size, &(m_variables[i][0]), true, nullptr)) {
              ErrorCheck::setError(8, "Cannot map host visible memomry");
            }
          }
        }
      }

      void addArray(std::string name, std::vector<int>& value) {
        if (m_variableNames.find(name) != m_variableNames.end()) {
          ErrorCheck::setError(8, "the variable allready exist");
          return;
        }
        int i = int(m_variables.size());
        m_variables.push_back(value);
        m_variableNames.insert(std::pair<std::string, int>(name, i));
      }

      void setArray(std::string name, std::vector<int>& value) {
        if (m_variableNames.find(name) == m_variableNames.end()) {
          ErrorCheck::setError(8, "variable does not exist");
          return;
        }
        int i = m_variableNames[name];
        std::vector<int> v = m_variables[i];
        if (v.size() != value.size()) {
          ErrorCheck::setError(8, "the stored variable and the new value are not compatible");
          return;
        }

        m_variables[i] = value;
        if (m_modified.size() > 0) {
          m_modified[i] = true;
        }
      }


      VkDestroyer(VkBuffer)                                     m_stagingBuffer;
      VkDestroyer(VkDeviceMemory)                               m_stagingBufferMemory;

      VkDestroyer(VkBuffer)                                     m_buffer;
      VkDestroyer(VkDeviceMemory)                               m_bufferMemory;

      VkDeviceSize                                              m_bufferSize = 0;
      std::map<std::string, int>                                m_variableNames;
      std::vector<std::vector<int>>                             m_variables;
      std::vector<bool>                                         m_modified;
      std::vector<std::pair<VkDeviceSize, VkDeviceSize>>        m_typeSizeOffset ;
    };
  }
}
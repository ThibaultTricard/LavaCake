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

			void end();

			void update(VkCommandBuffer& commandBuffer, bool all = true);

			VkBuffer& getHandle();

      ~UniformBuffer() {
        Device* d = Device::getDevice();
        VkDevice logical = d->getLogicalDevice();
        LavaCake::Buffer::DestroyBuffer(logical, *m_buffer);
        Memory::FreeMemoryObject(logical, *m_bufferMemory);

        LavaCake::Buffer::DestroyBuffer(logical, *m_stagingBuffer);
        Memory::FreeMemoryObject(logical, *m_stagingBufferMemory);
      }

    private :

			void copyToStageMemory(bool all = false);

			void addArray(std::string name, std::vector<int>& value);

			void setArray(std::string name, std::vector<int>& value);


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
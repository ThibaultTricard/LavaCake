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

      void addVariable(std::string name, Data* value) {
        std::vector<int> v = value->rawMemory();
        addArray(name, v);
      }

      void setVariable(std::string name, Data* value) {
        std::vector<int> v = value->rawMemory();
        setArray(name, v);
      }

      void addVariable(std::string name, std::vector<Data*>* value) {
        std::vector<int> v;
        for (size_t i = 0; i < value->size(); i++) {
          
          std::vector<int> temp = (*value)[i]->rawMemory();
          v.insert(v.end(), temp.begin(), temp.end());
        }
        addArray(name, v);
      }

      void setVariable(std::string name, std::vector<Data*>* value) {
        std::vector<int> v;
        for (size_t i = 0; i < value->size(); i++) {
          std::vector<int> temp = (*value)[i]->rawMemory();
          v.insert(v.end(), temp.begin(), temp.end());
        }
        setArray(name, v);
      }
      
      void addVariable(std::string name, int value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      void setVariable(std::string name, int value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        setArray(name, v);
      }


      void addVariable(std::string name, uint32_t value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      void setVariable(std::string name, uint32_t value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        setArray(name, v);
      }

      void addVariable(std::string name, float value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      void setVariable(std::string name, float value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        setArray(name, v);
      }


      void addVariable(std::string name, double value) {
        std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      void setVariable(std::string name, double value) {
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
        LavaCake::Core::DestroyBuffer(logical, *m_buffer);
        LavaCake::Core::FreeMemoryObject(logical, *m_bufferMemory);

        LavaCake::Core::DestroyBuffer(logical, *m_stagingBuffer);
        LavaCake::Core::FreeMemoryObject(logical, *m_stagingBufferMemory);
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
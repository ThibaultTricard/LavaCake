#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"
#include "Buffer.h"
#include "UniformBuffer.h"

namespace LavaCake {
  namespace Framework{

    /**
      \brief help manage the uniform buffer creation and updating their GPU memory.
      This class is mainly a dictionary of variable of different type that can be send to the the GPU.
    */
    class UniformBuffer {
    public :

      /**
       \brief Add a variable into the dictionary.
       \param name: the name of constant.
       \param value: the variable.
      */
      template<typename T>
      void addVariable(std::string name, T value) {
        std::vector<char> v = std::vector<char>(sizeof(value) / sizeof(char));
        std::memcpy(&v[0], &value, sizeof(value));
        addArray(name, v);
      }

      /**
        \brief set a variable into the dictionary
        \param name : the name of constant
        \param value : the variable
      */
      template<typename T>
      void setVariable(std::string name, T value) {
        std::vector<char> v = std::vector<char>(sizeof(value) / sizeof(char));
        std::memcpy(&v[0], &value, sizeof(value));
        setArray(name, v);
      }

      /**
        \brief notify the buffer that no new variable will be added
      */
			void end();

      /**
        \brief update the gpu memory of the uniform buffer
        \param cmdBuff: the command buffer used for this operation, must be in a recording state
      */
			void update(CommandBuffer& commandBuffer, bool all = true);



			VkBuffer& getHandle();

      ~UniformBuffer() {
      }

    private :

			void copyToStageMemory(bool all = false);

			void addArray(std::string name, std::vector<char>& value);

			void setArray(std::string name, std::vector<char>& value);


      Buffer                                                    m_buffer;
      Buffer                                                    m_stagingBuffer;

      VkDeviceSize                                              m_bufferSize = 0;
      std::map<std::string, uint32_t>                           m_variableNames;
      std::vector<std::vector<char>>                            m_variables;
      std::vector<bool>                                         m_modified;
      std::vector<std::pair<VkDeviceSize, VkDeviceSize>>        m_typeSizeOffset ;
    };
  }
}

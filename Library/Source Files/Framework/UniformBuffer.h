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
      void addVariable(const std::string& name, const T& value) {
        std::vector<std::byte> v = std::vector<std::byte>(sizeof(value) / sizeof(std::byte)); //TODO should not be required for some types
        std::memcpy(&v[0], &value, sizeof(value)); //TODO could be avoided
        addArray(name, v.data(), v.size());
      }

      template<typename TSpan>
      void addVariableSpan(const std::string& name, const TSpan& value_span) {
        addArray(name, value_span.data(), value_span.size());
      }

      /**
        \brief set a variable into the dictionary
        \param name : the name of constant
        \param value : the variable
      */
      template<typename T>
      void setVariable(const std::string& name, const T& value) {
        std::vector<std::byte> v = std::vector<std::byte>(sizeof(value) / sizeof(std::byte)); //TODO should not be required for some types
        std::memcpy(&v[0], &value, sizeof(value)); //TODO could be avoided
        setArray(name, v.data(), v.size());
      }

      template<typename TSpan>
      void setVariableSpan(const std::string& name, const TSpan& value_span) {
        setArray(name, value_span.data(), value_span.size());
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

      void addArray(const std::string& name, std::byte* data, unsigned int size); // Idea : directly accept a span as input, even of non byte type
      void setArray(const std::string& name, std::byte* data, unsigned int size);

      Buffer                                                    m_buffer;
      Buffer                                                    m_stagingBuffer;

      std::map<std::string, std::pair<uint32_t,uint32_t>>       m_variableNames; // maybe use a struct to name attribute ?
      std::vector<std::byte>                                    m_data;
      //std::vector<bool>                                         m_modified;
    };
  }
}

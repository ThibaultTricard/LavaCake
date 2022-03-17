#pragma once
#include <vector>
#include <map>
#include <span>
#include <iostream>
#include <string>
#include <span>
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"
#include "Buffer.h"
#include "UniformBuffer.h"
#include "ByteDictionary.h"

namespace LavaCake {
  namespace Framework {

    /**
          \brief help manage the uniform buffer creation and updating their GPU memory.
          This class is mainly a dictionary of variable of different type that can be send to the the GPU.
        */
    class UniformBuffer {
    public:
      ~UniformBuffer() = default;

      /**
           \brief Add a variable into the dictionary.
           \param name: the name of constant.
           \param value: the variable, either a simple data type of a contiguous range of simple data type.
          */
      template<typename T>
      void addVariable(const std::string& name, const T& value) {
        //    if constexpr(std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) {
        //      m_variables.addVariableRange(name, std::span { value });
        //    } else {
        m_variables.addVariableRange(name, std::span<const T, 1> { &value, 1 });
        //    }
      }

      template<typename T>
      void addVariable(const std::string& name, const std::vector<T>& value) {
        m_variables.addVariableRange(name, std::span{ value });
      }

      template<typename T, std::size_t N>
      void addVariable(const std::string& name, const std::array<T, N>& value) {
        m_variables.addVariableRange(name, std::span{ value });
      }

      template<typename T, std::size_t Extent>
      void addVariable(const std::string& name, const std::span<T, Extent>& value) {
        m_variables.addVariableRange(name, std::span{ value });
      }

      /**
            \brief set a variable into the dictionary.
            \param name : the name of constant
            \param value: the variable, either a simple data type of a contiguous range of simple data type.
          */
      template<typename T>
      void setVariable(const std::string& name, const T& value) {
        //    if constexpr(std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) {
        //      m_variables.setVariableRange(name, std::span{ value });
        //    } else {
        m_variables.setVariableRange(name, std::span<const T, 1>{ &value, 1 });
        //    }
      }

      template<typename T>
      void setVariable(const std::string& name, const std::vector<T>& value) {
        m_variables.setVariableRange(name, std::span{ value });
      }

      template<typename T, std::size_t N>
      void setVariable(const std::string& name, const std::array<T, N>& value) {
        m_variables.setVariableRange(name, std::span{ value });
      }

      template<typename T, std::size_t Extent>
      void setVariable(const std::string& name, const std::span<T, Extent>& value) {
        m_variables.setVariableRange(name, std::span{ value });
      }

      /**
            \brief notify the buffer that no new variable will be added
          */
      void end();

      /**
            \brief update the gpu memory of the uniform buffer
            \param cmdBuff: the command buffer used for this operation, must be in a recording state
          */
      void update(CommandBuffer& commandBuffer);

      const VkBuffer& getHandle() const;

      const Buffer& getBuffer() const;

    private:

      void copyToStageMemory(bool all = false);

      Buffer                                                    m_buffer;
      Buffer                                                    m_stagingBuffer;

      ByteDictionary                                            m_variables;
    };
  }
}

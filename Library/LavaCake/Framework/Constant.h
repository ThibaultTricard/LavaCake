#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include "Tools.h"
#include "ErrorCheck.h"
#include "Device.h"
#include "ByteDictionary.h"

namespace LavaCake {
  namespace Framework {
    /**
      \brief help manage pushConstant creation and sending them to shaders.
      This class is mainly a dictionary of variable of different type that can be pushed to a shader.
    */
    class PushConstant {
    public:
      ~PushConstant() = default;

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
      void push(VkCommandBuffer buffer, VkPipelineLayout layout, VkPushConstantRange range);

      uint32_t size();

    private:

      VkDeviceSize                                              m_bufferSize = 0;
      ByteDictionary                                            m_variables;
    };

  }
}

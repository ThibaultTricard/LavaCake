#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <span>
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
      ~UniformBuffer() = default;

      /**
       \brief Add a variable into the dictionary.
       \param name: the name of constant.
       \param value: the variable, either a simple data type of a contiguous range of simple data type.
      */
      template<typename T>
      void addVariable(const std::string& name, const T& value) {
        if constexpr(std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) {
          addVariableRange(name, std::span { value });
        } else {
          addVariableRange(name, std::span<const T,1> { &value, 1 });
        }
      }


      /**
        \brief set a variable into the dictionary.
        \param name : the name of constant
        \param value: the variable, either a simple data type of a contiguous range of simple data type.
      */
      template<typename T>
      void setVariable(const std::string& name, const T& value) {
        if constexpr(std::ranges::sized_range<T> && std::ranges::contiguous_range<T>) {
          setVariableRange(name, std::span{ value });
        } else {
          setVariableRange(name, std::span<const T,1>{ &value, 1 });
        }
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

			VkBuffer& getHandle();

    private :

			void copyToStageMemory(bool all = false);

      /**
       \brief Add a span of value into the dictionary.
       \param name: the name of constant.
       \param data: the span of value.
      */
      template<typename T, std::size_t TExtent = std::dynamic_extent>
      void addVariableRange(const std::string& name, const std::span<const T,TExtent> data);

      /**
        \brief set a span of value into the dictionary
        \param name : the name of constant
       \param data: the span of value.
      */
      template<typename T, std::size_t TExtent = std::dynamic_extent>
      void setVariableRange(const std::string& name, const std::span<T,TExtent> data);

      Buffer                                                    m_buffer;
      Buffer                                                    m_stagingBuffer;

      std::map<std::string, std::pair<uint32_t,uint32_t>>       m_variableNames; // maybe use a struct to name attribute ?
      std::vector<std::byte>                                    m_data;
      //std::vector<bool>                                         m_modified;
    };

    template<typename T, std::size_t TExtent>
    void UniformBuffer::addVariableRange(const std::string& name, const std::span<const T,TExtent> data) {
      uint32_t offset_data = (uint32_t)m_data.size();
      auto [it, inserted] = m_variableNames.try_emplace(std::string(name), offset_data, data.size_bytes());
      if (!inserted) [[unlikely]] {
        ErrorCheck::setError("The variable allready exist in this UniformBuffer",1);
        return;
      }

      m_data.resize(offset_data+data.size_bytes());
      std::memcpy(m_data.data()+offset_data, data.data(), data.size_bytes());
    }

    template<typename T, std::size_t TExtent>
    void UniformBuffer::setVariableRange(const std::string& name, const std::span<T,TExtent> data) {
      if (auto it = m_variableNames.find(name); it != m_variableNames.end()) [[likely]] {
        auto [offset_data, size_data] = it->second;
        if (size_data!=data.size_bytes()) [[unlikely]] {
          ErrorCheck::setError("The new value does not match the type of the one currently stored in this UniformBuffer",1);
          return;
        }

        std::memcpy(m_data.data()+offset_data, data.data(), data.size_bytes());

        //if (m_modified.size() > 0) {
        //  m_modified[i] = true;
        //}
        return;
      }
      ErrorCheck::setError("The variable does not exist in this UniformBuffer",1);
    }

  }
}

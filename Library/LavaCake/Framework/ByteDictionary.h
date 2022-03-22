#pragma once

#include <vector>
#include <map>
#include <string>
#include <span>
#include <cstring>

#include "ErrorCheck.h"

namespace LavaCake {
  namespace Framework {
    /**
      \brief help manage pushConstant creation and sending them to shaders.
      This class is mainly a dictionary of variable of different type that can be pushed to a shader.
    */
    class ByteDictionary {
    public:
      ~ByteDictionary() = default;

      const std::vector<std::byte>& data() const { return m_data; };

      /**
       \brief Add a span of value into the dictionary.
       \param name: the name of constant.
       \param data: the span of value.
      */
      template<typename T, std::size_t TExtent = std::dynamic_extent>
      void addVariableRange(const std::string& name, const std::span<const T, TExtent> data);

      /**
        \brief set a span of value into the dictionary
        \param name : the name of constant
       \param data: the span of value.
      */
      template<typename T, std::size_t TExtent = std::dynamic_extent>
      void setVariableRange(const std::string& name, const std::span<T, TExtent> data);


    private:
      std::map<std::string, std::pair<uint32_t, uint32_t>>       m_variableNames; // maybe use a struct to name attribute ?
      std::vector<std::byte>                                    m_data;
    };

    template<typename T, std::size_t TExtent>
    void ByteDictionary::addVariableRange(const std::string& name, const std::span<const T, TExtent> data) {
      uint32_t offset_data = (uint32_t)m_data.size();
      auto [it, inserted] = m_variableNames.try_emplace(std::string(name), offset_data, data.size_bytes());
      if (!inserted) [[unlikely]] {
        ErrorCheck::setError("The variable allready exist in this UniformBuffer",1);
        return;
      }

      m_data.resize(offset_data + data.size_bytes());
      std::memcpy(m_data.data() + offset_data, data.data(), data.size_bytes());
    }

    template<typename T, std::size_t TExtent>
    void ByteDictionary::setVariableRange(const std::string& name, const std::span<T, TExtent> data) {
      if (auto it = m_variableNames.find(name); it != m_variableNames.end()) [[likely]] {
        auto [offset_data, size_data] = it->second;
        if (size_data != data.size_bytes()) [[unlikely]] {
          ErrorCheck::setError("The new value does not match the type of the one currently stored in this UniformBuffer",1);
          return;
        }

        std::memcpy(m_data.data() + offset_data, data.data(), data.size_bytes());

        //if (m_modified.size() > 0) {
        //  m_modified[i] = true;
        //}
        return;
      }
      ErrorCheck::setError("The variable does not exist in this UniformBuffer", 1);
    }


  }
}

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
	namespace Framework {
  /**
   Push Constant class :
   \brief help manage pushConstant creation and sending them to shaders.
    This class is mainly a dictionary of variable of different type that can be pushed to a shader.
  */
		class PushConstant {
		public:

			/**
       \brief Add a variable into the dictionary.
       \param name : the name of constant.
       \param value : the variable.
			*/
			template<typename T>
			void addVariable(std::string name, T& value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(T));
				addArray(name, v);
			}

      /**
      \brief set a variable into the dictionary
      \param name : the name of constant
      \param value : the variable
      */
			template<typename T>
			void setVariable(std::string name, T& value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(value));
				setArray(name, v);
			}

			void push(VkCommandBuffer buffer, VkPipelineLayout layout, VkShaderStageFlags flag);

			uint32_t size();

		private:


			void addArray(std::string name, std::vector<int>& value);
			void setArray(std::string name, std::vector<int>& value);


			VkDeviceSize                                              m_bufferSize = 0;
			std::map<std::string, int>                                m_variableNames;
			std::vector<std::vector<int>>                             m_variables;
			std::vector<bool>                                         m_modified;
			std::vector<std::pair<VkDeviceSize, VkDeviceSize>>        m_typeSizeOffset;
			uint32_t																									m_size = 0;
		};
	}
}

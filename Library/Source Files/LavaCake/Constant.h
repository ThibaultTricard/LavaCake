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
		class PushConstant {
		public:

			/**
			*add a Variable into the PushConstant
			*/
			template<typename T>
			void addVariable(std::string name, T value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(value));
				addArray(name, v);
			}


			/**
			* change one of value in the push constant using it's name 
			*/
			template<typename T>
			void setVariable(std::string name, T value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(value));
				setArray(name, v);
			}

			/**
			*push the constant into the command buffer
			*/
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
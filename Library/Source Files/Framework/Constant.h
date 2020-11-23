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
			void addVariable(std::string name, Data* value) {
				std::vector<int> v = value->rawMemory();
				addArray(name, v);
			}

			void setVariable(std::string name, Data* value) {
				std::vector<int> v = value->rawMemory();
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
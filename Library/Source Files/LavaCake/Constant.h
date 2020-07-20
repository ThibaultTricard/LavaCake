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
			template<typename T>
			void addVariable(std::string name, T value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(value));
				addArray(name, v);
			}

			template<typename T>
			void setVariable(std::string name, T value) {
				std::vector<int> v = std::vector<int>(sizeof(value) / sizeof(int));
				std::memcpy(&v[0], &value, sizeof(value));
				setArray(name, v);
			}

			void push(VkCommandBuffer buffer, VkPipelineLayout layout, VkShaderStageFlags flag) {
				int offset = 0;
				for (int i = 0; i < m_variables.size(); i++) {
					LavaCake::vkCmdPushConstants(buffer, layout, flag, offset, sizeof(int)* m_variables[i].size(), &(m_variables[i][0]));
					offset += sizeof(int)* m_variables[i].size();
				}
			}

			uint32_t size() {
				return m_size;
			}

		private:

			void addArray(std::string name, std::vector<int>& value) {
				if (m_variableNames.find(name) != m_variableNames.end()) {
					ErrorCheck::setError(8, "the variable allready exist");
					return;
				}
				int i = int(m_variables.size());
				m_variables.push_back(value);
				m_variableNames.insert(std::pair<std::string, int>(name, i));
				m_size += value.size();
			}

			void setArray(std::string name, std::vector<int>& value) {
				if (m_variableNames.find(name) == m_variableNames.end()) {
					ErrorCheck::setError(8, "variable does not exist");
					return;
				}
				int i = m_variableNames[name];
				std::vector<int> v = m_variables[i];
				if (v.size() != value.size()) {
					ErrorCheck::setError(8, "the stored variable and the new value are not compatible");
					return;
				}

				m_variables[i] = value;
				if (m_modified.size() > 0) {
					m_modified[i] = true;
				}
			}


			VkDeviceSize                                              m_bufferSize = 0;
			std::map<std::string, int>                                m_variableNames;
			std::vector<std::vector<int>>                             m_variables;
			std::vector<bool>                                         m_modified;
			std::vector<std::pair<VkDeviceSize, VkDeviceSize>>        m_typeSizeOffset;
			uint32_t																									m_size = 0;
		};
	}
}
#include "Constant.h"

namespace LavaCake {
	namespace Framework {

		void PushConstant::push(VkCommandBuffer buffer, VkPipelineLayout layout, VkShaderStageFlags flag) {
			uint32_t offset = 0;
			for (uint32_t i = 0; i < static_cast<uint32_t>(m_variables.size()); i++) {
				uint32_t size = static_cast<uint32_t>(sizeof(int) * m_variables[i].size());
				LavaCake::vkCmdPushConstants(buffer, layout, flag, offset, size, &(m_variables[i][0]));
				offset += size;
			}
		}

		uint32_t PushConstant::size() {
			return m_size;
		}

		void PushConstant::addArray(std::string name, std::vector<int>& value) {
			if (m_variableNames.find(name) != m_variableNames.end()) {
				ErrorCheck::setError((char*)"Variable name allready taken in this PushConstant");
				return;
			}
			int i = int(m_variables.size());
			m_variables.push_back(value);
			m_variableNames.insert(std::pair<std::string, int>(name, i));
			m_size += static_cast<uint32_t>(value.size());
		}

		void PushConstant::setArray(std::string name, std::vector<int>& value) {
			if (m_variableNames.find(name) == m_variableNames.end()) {
				ErrorCheck::setError((char*)"Variable name does not exist in this PushConstant");
				return;
			}
			int i = m_variableNames[name];
			std::vector<int> v = m_variables[i];
			if (v.size() != value.size()) {
				ErrorCheck::setError((char*)"The new value does not match the type of the one currently stored in this PushConstant");
				return;
			}

			m_variables[i] = value;
			if (m_modified.size() > 0) {
				m_modified[i] = true;
			}
		}
	}
}

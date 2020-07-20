#pragma once
#include "AllHeaders.h"
#include "VulkanDestroyer.h"
#include "Queue.h"
#include "Device.h"

namespace LavaCake {
	namespace Framework {
		class VertexBuffer {
		public:
			VertexBuffer(std::vector <LavaCake::Helpers::Mesh::Mesh*> m, std::vector<int> dataDescription, uint32_t binding = 0, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

			void allocate(LavaCake::Framework::Queue& queue, VkCommandBuffer& commandBuffer);

			std::vector <LavaCake::Helpers::Mesh::Mesh*> getMeshs();
			
			VkBuffer& getBuffer();
			
			std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions();

			std::vector<VkVertexInputBindingDescription>& getBindingDescriptions();
			

		private :
			std::vector<VkVertexInputAttributeDescription>			m_attributeDescriptions;
			std::vector<VkVertexInputBindingDescription>				m_bindingDescriptions;
			VkDestroyer(VkBuffer)																m_buffer;
			VkDestroyer(VkDeviceMemory)													m_bufferMemory;
			std::vector<float>*																	m_vertices;
			std::vector <LavaCake::Helpers::Mesh::Mesh*>				m_meshs;
		};

	}
}
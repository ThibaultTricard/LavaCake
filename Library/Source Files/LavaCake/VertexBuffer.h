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

			void allocate(VkQueue& queue, VkCommandBuffer& commandBuffer);

			std::vector <LavaCake::Helpers::Mesh::Mesh*> getMeshs();
			
			VkBuffer& getBuffer();
			
			VkBuffer& getIndexBuffer();

			void swapMeshes(std::vector <LavaCake::Helpers::Mesh::Mesh*>	meshes);

			std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions();

			std::vector<VkVertexInputBindingDescription>& getBindingDescriptions();
			
			bool isIndexed();

		private :
			std::vector<VkVertexInputAttributeDescription>			m_attributeDescriptions;
			std::vector<VkVertexInputBindingDescription>				m_bindingDescriptions;
			VkDestroyer(VkBuffer)																m_buffer;
			VkDestroyer(VkDeviceMemory)													m_bufferMemory;
			std::vector<float>*																	m_vertices;
			VkDestroyer(VkBuffer)																m_indexBuffer;
			VkDestroyer(VkDeviceMemory)													m_indexBufferMemory;
			std::vector<uint16_t>*															m_indices;
			std::vector <LavaCake::Helpers::Mesh::Mesh*>				m_meshs;
			bool																								m_indexed;
		};

	}
}
#include "VertexBuffer.h"
#include "CommandBuffer.h"

namespace LavaCake {
	namespace Framework {

		VertexBuffer::VertexBuffer(const std::vector<LavaCake::Geometry::Mesh_t*>& m, uint32_t binding ,VkVertexInputRate inputRate) {

			m_topology = m[0]->getTopology();
			m_stride = (uint32_t)m[0]->vertexSize();
			m_attributeDescriptions = m[0]->VkDescription();
			m_vertexBuffer = std::make_shared<Buffer>();
			m_indexBuffer = std::make_shared<Buffer>();
			for (size_t t = 0; t < m_attributeDescriptions.size(); t++) {
				m_attributeDescriptions[t].binding = binding;
			}

			m_bindingDescriptions.push_back(
				{
							binding,
							uint32_t(m_stride * sizeof(float)),
							inputRate
				});

			swapMeshes(m);
		};

		void VertexBuffer::allocate(Queue& queue, CommandBuffer& cmdBuff, VkBufferUsageFlags otherUsage) {
			if (m_vertices.size() == 0)return;

			m_vertexBuffer->allocate(queue, cmdBuff, m_vertices, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT| otherUsage), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_FORMAT_R32_SFLOAT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

			if (m_indexed) {

				m_indexBuffer->allocate(queue, cmdBuff, m_indices, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT| otherUsage), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_FORMAT_R32_UINT, VK_ACCESS_INDEX_READ_BIT);

			}
      
			m_verticesSize =(uint32_t)m_vertices.size();
			m_vertices.clear();
			m_indicesSize  = (uint32_t)m_indices.size();
			m_indices.clear();
		}

		
		
		std::shared_ptr<Buffer> VertexBuffer::getVertexBuffer() {
			return m_vertexBuffer;
		}

		std::shared_ptr<Buffer> VertexBuffer::getIndexBuffer() {
			return m_indexBuffer;
		}

		std::vector<VkVertexInputAttributeDescription>& VertexBuffer::getAttributeDescriptions() {
			return m_attributeDescriptions;
		}

		std::vector<VkVertexInputBindingDescription>& VertexBuffer::getBindingDescriptions() {
			return m_bindingDescriptions;
		}

		bool VertexBuffer::isIndexed() {
			return m_indexed;
		};

		
		void VertexBuffer::swapMeshes(std::vector<LavaCake::Geometry::Mesh_t*>				m) {
			if (m_topology == m[0]->getTopology()) {
				m_vertices = std::vector<float>(m[0]->vertices());
				m_indices = std::vector<uint32_t>(m[0]->indices());

				m_indexed = m[0]->isIndexed();
				for (unsigned int i = 1; i < m.size(); i++) {
					if (m_indexed) {
						for (size_t j = 0; j < m[i]->indices().size(); j++) {
							m_indices.push_back(m[i]->indices()[j] + uint32_t(m_vertices.size()/m_stride));
						}
					}
					m_vertices.insert(m_vertices.end(), m[i]->vertices().begin(), m[i]->vertices().end());
				}
			}

		};
	}
}

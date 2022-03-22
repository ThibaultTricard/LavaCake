#include "VertexBuffer.h"
#include "CommandBuffer.h"

namespace LavaCake {
  namespace Framework {

    VertexBuffer::VertexBuffer(
      const  Queue& queue,
      CommandBuffer& cmdBuff,
      const std::vector<LavaCake::Geometry::Mesh_t*>& m,
      uint32_t binding,
      VkVertexInputRate inputRate,
      VkBufferUsageFlags otherUsage) {

      m_topology = m[0]->getTopology();
      m_stride = (uint32_t)m[0]->vertexSize();
      m_attributeDescriptions = m[0]->VkDescription();
      for (size_t t = 0; t < m_attributeDescriptions.size(); t++) {
        m_attributeDescriptions[t].binding = binding;
      }

      m_bindingDescriptions.push_back(
        {
              binding,
              uint32_t(m_stride * sizeof(float)),
              inputRate
        });


      std::vector<float> vertices = std::vector<float>(m[0]->vertices());
      std::vector<uint32_t> indices = std::vector<uint32_t>(m[0]->indices());
      if (m_topology == m[0]->getTopology()) {
        vertices = std::vector<float>(m[0]->vertices());
        indices = std::vector<uint32_t>(m[0]->indices());

        m_indexed = m[0]->isIndexed();
        for (unsigned int i = 1; i < m.size(); i++) {
          if (m_indexed) {
            for (size_t j = 0; j < m[i]->indices().size(); j++) {
              indices.push_back(m[i]->indices()[j] + uint32_t(vertices.size() / m_stride));
            }
          }
          vertices.insert(vertices.end(), m[i]->vertices().begin(), m[i]->vertices().end());
        }
      }


      if (vertices.size() == 0)return;

      m_vertexBuffer = std::make_shared<Buffer>(queue, cmdBuff, vertices, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsage), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_FORMAT_R32_SFLOAT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

      if (m_indexed) {

        m_indexBuffer = std::make_shared<Buffer>(queue, cmdBuff, indices, (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsage), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_FORMAT_R32_UINT, VK_ACCESS_INDEX_READ_BIT);

      }

      m_verticesSize = (uint32_t)vertices.size();
      m_indicesSize = (uint32_t)indices.size();
    };


    std::shared_ptr<Buffer> VertexBuffer::getVertexBuffer() const {
      return m_vertexBuffer;
    }

    std::shared_ptr<Buffer> VertexBuffer::getIndexBuffer() const {
      return m_indexBuffer;
    }

    const std::vector<VkVertexInputAttributeDescription>& VertexBuffer::getAttributeDescriptions() const {
      return m_attributeDescriptions;
    }

    const std::vector<VkVertexInputBindingDescription>& VertexBuffer::getBindingDescriptions() const {
      return m_bindingDescriptions;
    }

    bool VertexBuffer::isIndexed() const {
      return m_indexed;
    };


  }
}

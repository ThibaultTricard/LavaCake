#include "./format.h"
namespace LavaCake {
  namespace Geometry {

    template <topology T>
    class Mesh {
      std::vector<float>      m_vertices;
      vertexFormat            m_format;
      size_t                  m_vertexSize;

    public : 

      Mesh(vertexFormat format) {
        m_vertexSize = size(format);
      }

      void appendVertex(std::vector<float> vertex) {
        if(vertex.size() == m_vertexSize)
          m_vertices.insert(m_vertices.end(), vertex.begin(), vertex.end());
      }
      
    };

    template <topology T>
    class IndexedMesh : public Mesh {
      std::vector<uint32_t> m_indices;
    
    public :
      void appendIndex(uint32_t index) {
        indices.push_back(index);
      }

    };




    typedef Mesh<POINT> TriangleMesh;
    typedef Mesh<LINE> LineMesh;
    typedef IndexedMesh<LINE> LineIndexedMesh;
    typedef Mesh<TRIANGLE> TriangleMesh;
    typedef IndexedMesh<TRIANGLE> TriangleIndexedMesh;
  }
}
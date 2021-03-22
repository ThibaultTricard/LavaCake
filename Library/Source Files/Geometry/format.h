#pragma once 

#include <vector>
#include "AllHeaders.h"

namespace LavaCake {
  namespace Geometry {

  /**
   \brief Enum  : primitive format
   Format of variable allowed in a mesh
  */
    enum primitiveFormat {
      POS2,  /*!< a 2D position */
      POS3,  /*!< a 3D position */
      NORM3, /*!< a 3D vector representing a normal */
      UV,    /*!< a uv coordinate*/
      COL3,  /*!< a RGB value */
      COL4,  /*!< a RGBA value */
      F1,    /*!< a 1 dimensional */
      F2,    /*!< a 2 dimensional float */
      F3,    /*!< a 3 dimensional float */
      F4     /*!< a 4 dimensional float */
    };


    size_t static toSize(primitiveFormat f) {
      switch (f) {
      case POS2:
        return 2;
        break;
      case POS3:
        return 3;
        break;
      case NORM3:
        return 3;
        break;
      case UV:
        return  2;
        break;
      case COL3:
        return 3;
        break;
      case COL4:
        return 4;
        break;
      case F1:
        return 1;
        break;
      case F2:
        return 2;
        break;
      case F3:
        return 3;
        break;
      case F4:
        return 4;
        break;
      default :
        return 0;
      }
    }


/**
 \brief Class vertexFormat : help define the stride of mesh
 */
    class vertexFormat {
    private : 
      std::vector<primitiveFormat> m_description;
      size_t m_size = 0;
      std::vector<VkVertexInputAttributeDescription> m_vulkanDescription;

      
    public :
      /**
       \brief Default constructor
       */
      vertexFormat() {};
      
      /**
       \brief Create a vertex format, with a lis of primitive format
       \param description a list of primitive formats that define the stride
       */
      vertexFormat(std::vector<primitiveFormat> description) {
        m_description = description;
        for (size_t t = 0; t < m_description.size(); t++) {
          size_t s = toSize(m_description[t]);

          VkFormat f = VK_FORMAT_UNDEFINED;
          if (s == 1) {
            f = VK_FORMAT_R32_SFLOAT;
          }
          else if (s == 2) {
            f = VK_FORMAT_R32G32_SFLOAT;
          }
          else if (s == 3) {
            f = VK_FORMAT_R32G32B32_SFLOAT;
          }
          else if (s == 4) {
            f = VK_FORMAT_R32G32B32A32_SFLOAT;
          }

          m_vulkanDescription.push_back({
            uint32_t(t),
            0,
            f,
            uint32_t(m_size * sizeof(float))
            });
          m_size += s;
        }
      
      
      }
      
      std::vector<primitiveFormat>& description() {
        return m_description;
      }

      size_t size() {
        return m_size;
      }

      std::vector<VkVertexInputAttributeDescription> VkDescription() {
        return m_vulkanDescription;
      }

    };


    static vertexFormat P3 = vertexFormat({POS3});
    static vertexFormat PC3 = vertexFormat({ POS3,COL3});
    static vertexFormat PN3 = vertexFormat({POS3,NORM3} );
    static vertexFormat P3UV = vertexFormat({ POS3,UV });
    static vertexFormat PN3UV = vertexFormat({POS3,NORM3,UV} );
    static vertexFormat PNC3 = vertexFormat({POS3,NORM3,COL3} );

    enum topology {
      POINT,
      LINE,
      TRIANGLE,
    };
  }
}

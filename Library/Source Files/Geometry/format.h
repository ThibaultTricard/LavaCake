#pragma once 
#include <vector>

namespace LavaCake {
  namespace Geometry {

    enum primitiveFormat {
      POS2,
      POS3,
      POS4,
      NORM3,
      NORM4,
      UV,
      COL3,
      COL4,
      F1,
      F2,
      F3,
      F4
    };

    struct vertexFormat_t{
      std::vector<primitiveFormat> description;
    };
    typedef vertexFormat_t vertexFormat;

   
    size_t size(vertexFormat vf) {
      size_t s = 0;
      for (size_t t = 0; t < vf.description.size(); t++) {
        switch (vf.description[t]) {
        case POS2 :
          s += 2;
          break;
        case POS3:
          s += 3;
          break;
        case POS4:
          s += 4;
          break;
        case NORM3:
          s += 3;
          break;
        case NORM4:
          s += 4;
          break;
        case UV:
          s += 2;
          break;
        case COL3:
          s += 3;
          break;
        case COL4:
          s += 4;
          break;
        case F1:
          s += 1;
          break;
        case F2:
          s += 2;
          break;
        case F3:
          s += 3;
          break;
        case F4:
          s += 4;
          break;
        }
      }
      return s;
    }

    vertexFormat P3 = { {POS3} };
    vertexFormat PN3 = { {POS3,NORM3} };
    vertexFormat PN3UV = { {POS3,NORM3,UV} };
    vertexFormat PNC3 = { {POS3,NORM3,COL3} };

    enum topology {
      POINT,
      LINE,
      TRIANGLE,
    };
  }
}
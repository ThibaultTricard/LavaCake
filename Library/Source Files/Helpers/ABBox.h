#pragma once
#include <array>
#include "AllHeaders.h"
#include "Math/basics.h"

namespace LavaCake {
  namespace Helpers {
  /**
   *Class ABBox :
   *\brief Represent a N dimentionals Bounding box
   *\tparam N The number of dimention
   */
    template<uint8_t N>
    class ABBox{
    public:
      
      
      /**
       \brief default constructor
      */
      ABBox(){};
      
      /**
       \brief Create a bounding box with it's min and max point
       \param min the min point of the bounding box
       \param max the max point of the bounding box
       */
      ABBox(std::array<float,N> min,std::array<float,N> max){
        m_min = min;
        m_max = max;
      }
      
      /**
       \brief get the min point of the bounding box
       \return a std array representing the min point of the bouding box
      */
      std::array<float, N> A(){
        return m_min;
      }
      
      /**
       \brief get the max point of the bounding box
       \return a std array representing the max point of the bouding box
       */
      std::array<float, N> B(){
        return m_max;
      }
      
      /**
       \brief compute the bounding box diagonal
       \return a std array representing the diagonal of the bounding box
       */
      std::array<float, N> diag(){
        return m_max-m_min;
      }
      
    private:
      
      std::array<float,N> m_min;
      std::array<float,N> m_max;
      
    };
  }
}

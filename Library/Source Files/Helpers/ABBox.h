#pragma once
#include <array>
#include "AllHeaders.h"

namespace LavaCake {
  namespace Helpers {
    template<uint8_t N>
    class ABBox{
    public:
      
      ABBox(){};
      
      ABBox(std::array<float,N> min,std::array<float,N> max){
        m_min = min;
        m_max = max;
      }
      
      std::array<float, N> min(){
        return m_min;
      }
      
      std::array<float, N> diag(){
        return m_max-m_min;
      }
      
      std::array<float, N> max(){
        return m_max;
      }
      
    private:
      
      std::array<float,N> m_min;
      std::array<float,N> m_max;
      
    };
  }
}

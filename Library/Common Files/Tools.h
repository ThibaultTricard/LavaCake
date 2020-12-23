// MIT License
//
// Copyright( c ) 2017 Packt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Vulkan Cookbook
// ISBN: 9781786468154
// © Packt Publishing Limited
//
// Author:   Pawel Lapinski
// LinkedIn: https://www.linkedin.com/in/pawel-lapinski-84522329
//
// Tools

#ifndef TOOLS
#define TOOLS

#include <tuple>
#include "Common.h"

namespace LavaCake {

  typedef float radian;
  typedef float degree;

  class Data {
  public:
    virtual std::vector<int> rawMemory() = 0;
  };

  template<typename T, unsigned char N>
  class v : public Data {
  private:
    std::array<T, N> data;

  public:
    v() {
      for (unsigned char i = 0; i < N; i++) {
        data[i] = T(0);
      }
    }

    v(const std::array<T, N>& d) {
      data = std::array<T, N>(d);
    }

    std::vector<int> rawMemory() override {
      std::vector<int> mem(length() * sizeof(T) / sizeof(int));
      std::memcpy(&mem[0], &data, sizeof(int) * mem.size());
      return mem;
    }

    size_t length() {
      return size_t(N);
    }

    T& operator [](const int idx) {
      return data[idx];
    }

    T operator [](const int idx) const {
      return data[idx];
    }
  };

  template<typename T, unsigned char N>
  v<T, N> operator*(const v<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return v<T, N>(d);
  }

  template<typename T, unsigned char N>
  v<T, N> operator*(const T a, const v<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return v<T, N>(d);
  }

  template<typename T, unsigned char N>
  v<T, N> operator-(const v<T, N> a, const v<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] - b[i];
    }
    return v<T, N>(d);
  }

  template<typename T, unsigned char N>
  v<T, N> operator+(const v<T, N> a, const v<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] + b[i];
    }
    return v<T, N>(d);
  }


  template<typename T, unsigned char N>
  class m : public Data {
    std::array<T, N* N> data;
  public:

    m() {
      for (unsigned char i = 0; i < N * N; i++) {
        data[i] = T(0);
      }
    }

    m(const std::array<T, N* N>& d) {
      data = std::array<T, N* N>(d);
    }

    std::vector<int> rawMemory() override {
      std::vector<int> mem(length() * sizeof(T) / sizeof(int));
      std::memcpy(&mem[0], &data, sizeof(int) * mem.size());
      return mem;
    }

    T& operator [](const int idx) {
      return data[idx];
    }

    T operator [](const int idx) const {
      return data[idx];
    }
    size_t length() {
      return size_t(N * N);
    }

    m<T, N> operator+(const m<T, N> a) {
      std::array<T, N* N> d = std::array<T, N* N>(data);
      for (unsigned char i = 0; i < N; i++) {
        d[i] += a[i];
      }
      return m<T, N>(d);
    }

    m<T, N> operator-(const m<T, N> a) {
      std::array<T, N* N> d = std::array<T, N* N>(data);
      for (unsigned char i = 0; i < N; i++) {
        d[i] -= a[i];
      }
      return m<T, N>(d);
    }

  };





  using vec2f = v<float, 2>;
  using vec2d = v<double, 2>;
  using vec2i = v<int, 2>;
  using vec2u = v<uint32_t, 2>;
  using vec2b = v<bool, 2>;

  using vec3f = v<float, 3>;
  using vec3d = v<double, 3>;
  using vec3i = v<int, 3>;
  using vec3u = v<uint32_t, 3>;
  using vec3b = v<bool, 3>;

  using vec4f = v<float, 4>;
  using vec4d = v<double, 4>;
  using vec4i = v<int, 4>;
  using vec4u = v<uint32_t, 3>;
  using vec4b = v<bool, 4>;

  using mat2 = m<float, 2>;
  using mat3 = m<float, 3>;
  using mat4 = m<float, 4>;

  bool GetBinaryFileContents(std::string const& filename,
    std::vector<unsigned char>& contents);

  radian Deg2Rad(degree value);

  float Dot(vec3f const& left,
    vec3f const& right);

  vec3f Cross(vec3f const& left,
    vec3f const& right);


  vec3f Normalize(vec3f const& vector);


  vec3f operator* (vec3f const& left,
    mat4 const& right);


  bool operator== (vec3f const& left,
    vec3f const& right);


  float Dot(vec3f const & left,
    vec3f const & right );

  vec3f Cross( vec3f const & left,
                 vec3f const & right );

  vec3f Normalize( vec3f const & vector );

  

} 



#endif // TOOLS
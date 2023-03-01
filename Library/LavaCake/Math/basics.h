#pragma once

#include <cmath>
#include <array>
#include <tuple>






namespace LavaCake {

  template<typename T, unsigned long size>
  struct vec{
    std::array<T, size> data;

    vec(){
      data.fill(T(0));
    }

    vec(const std::array<T, size>& v){
      data = std::array<T, size>(v);
    }

    vec(const T v[size]){
      for (size_t i = 0; i < size; i++)
      {
        data[i] = v[i];
      }
    }

    const T& operator[](const int index) const{
      return data[index];
    }

    T& operator[](const int index){
      return data[index];
    }

    template<unsigned long N>
    vec<T,N> operator[](const std::array<int,N>& indices){
      vec<T,N> res;
      for(unsigned i =0; i < N; i++){
        res[i] = data[indices[i]];
      }
      return res;
    }

    template<unsigned long N>
    vec<T,N> operator[](const vec<int,N>& indices){
      vec<T,N> res;
      for(unsigned i =0; i < N; i++){
        res[i] = data[indices[i]];
      }
      return res;
    }

    
  };

  template<typename T, unsigned long ColumnSize, unsigned long RowSize>
  struct mat{
    std::array<vec<T, ColumnSize>,RowSize> data;
    

    mat(){
      for(int j = 0; j <RowSize; j ++){
        data[j].data.fill(T(0));
      }
    }

    mat(const std::array<vec<T, ColumnSize>,RowSize>& m){
      data = std::array<vec<T, ColumnSize>,RowSize>(m);
    }

    mat(const std::array<T,ColumnSize*RowSize> m){
      for(int i = 0; i <ColumnSize; i ++){
        for(int j = 0; j <RowSize; j ++){
          data[j][i] = m[j*ColumnSize+i];
        };
      };
    }

    mat(const T m[ColumnSize*RowSize]){
      for(int i = 0; i <ColumnSize; i ++){
        for(int j = 0; j <RowSize; j ++){
          data[j][i] = m[j*ColumnSize+i];
        };
      };
    }

    const vec<T, ColumnSize>& operator[](int const index) const{
      return data[index];
    }

    vec<T, ColumnSize>& operator[](int const index){
      return data[index];
    }

  };
  
  using mat2f = mat<float,2,2>;
  using mat3f = mat<float,3,3>;
  using mat4f = mat<float,4,4>;

  using mat2d = mat<double,2,2>;
  using mat3d = mat<double,3,3>;
  using mat4d = mat<double,4,4>;
  

  using vec2f = vec<float, 2>;
  using vec2d = vec<double, 2>;
  using vec2i = vec<int, 2>;
  using vec2u = vec<unsigned int, 2>;
  using vec2b = vec<bool, 2>;

  using vec3f = vec<float, 3>;
  using vec3d = vec<double, 3>;
  using vec3i = vec<int, 3>;
  using vec3u = vec<unsigned int, 3>;
  using vec3b = vec<bool, 3>;

  using vec4f = vec<float, 4>;
  using vec4d = vec<double, 4>;
  using vec4i = vec<int, 4>;
  using vec4u = vec<unsigned int, 4>;
  using vec4b = vec<bool, 4>;

}
  

template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, M,N> transpose (const LavaCake::mat<T, N,M>& matrix){
  LavaCake::mat<T, M,N> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[i][j] = matrix[j][i];
    }
  }
  return res;
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator*(const LavaCake::vec<T, N>& b, const T a) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = b[i] * a;
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator*(const T a, const LavaCake::vec<T, N>& b) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = b[i] * a;
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator*(const LavaCake::vec<T, N>& a, const LavaCake::vec<T, N>& b) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = b[i] * a[i];
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator/(const LavaCake::vec<T, N>& a, const LavaCake::vec<T, N>& b) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = a[i] / b[i];
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator/(const LavaCake::vec<T, N>& b, const T a) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = b[i] / a;
  }
  return LavaCake::vec<T, N>(d);
}


template<typename T, unsigned long N>
LavaCake::vec<T, N> operator-(const LavaCake::vec<T, N> a, const LavaCake::vec<T, N> b) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = a[i] - b[i];
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator-(const LavaCake::vec<T, N> v) {
  LavaCake::vec<T, N> res = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    res[i] = -v[i];
  }
  return LavaCake::vec<T, N>(res);
}

template<typename T, unsigned long N>
LavaCake::vec<T, N> operator+(const LavaCake::vec<T, N> a, const LavaCake::vec<T, N> b) {
  LavaCake::vec<T, N> d = LavaCake::vec<T, N>();
  for (unsigned char i = 0; i < N; i++) {
    d[i] = a[i] + b[i];
  }
  return LavaCake::vec<T, N>(d);
}

template<typename T, unsigned long N>
T dot(LavaCake::vec<T, N> const & left,
      LavaCake::vec<T, N> const & right ){
  T sum = static_cast<T>(0);
  for(unsigned long u = 0; u< N ; u++){
    sum +=  left[u] * right[u];
  }
  return sum;
};

template<typename T, unsigned long N>
LavaCake::vec<T, N> normalize(LavaCake::vec<T, N> const vector) {
  T length = dot(vector,vector);
  if (length >0)
    return vector/T(sqrt(length));
  return vector;
}


template<typename T, unsigned long N1,unsigned long M1N2, unsigned long M2>
LavaCake::mat<T, N1,M2> operator*(LavaCake::mat<T, N1,M1N2> left, LavaCake::mat<T, M1N2,M2> right){
  LavaCake::mat<T, N1,M2> res;
  for(unsigned long i = 0; i < N1; i++){
    for(unsigned long j = 0; j < M2; j++){
      for(unsigned long k = 0; k < M1N2; k++){
        res[j][i] += left[k][i] * right[j][k];
      }
    }
  }
  return res;
}

template<typename T, unsigned long N1,unsigned long vecSize>
LavaCake::vec<T, vecSize> operator*(
  const LavaCake::mat<T, N1,vecSize>& left,
  const LavaCake::vec<T, vecSize>& right){
  auto tmp =left * LavaCake::mat<T,vecSize,1>(right.data);

  

  return tmp[0];
}



template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, N,M> operator-(const LavaCake::mat<T, N,M>& matrix){
  LavaCake::mat<T, N,M> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[j][i] = -matrix[j][i];
    }
  }
  return res;
}

template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, N,M> operator-(const LavaCake::mat<T, N,M>& left, const LavaCake::mat<T, N,M>& right){
  LavaCake::mat<T, N,M> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[j][i] = left[j][i]-right[j][i];
    }
  }
  return res;
}

template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, N,M> operator+(const LavaCake::mat<T, N,M>& left, const LavaCake::mat<T, N,M>& right){
  LavaCake::mat<T, N,M> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[j][i] = left[j][i]+right[j][i];
    }
  }
  return res;
}
 

template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, N,M> operator*(const T& left, const LavaCake::mat<T, N,M>& right){
  LavaCake::mat<T, N,M> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[j][i] = left*right[j][i];
    }
  }
  return res;
}

template<typename T, unsigned long N,unsigned long M>
LavaCake::mat<T, N,M> operator*(const  LavaCake::mat<T, N,M>& left, const T& right){
  LavaCake::mat<T, N,M> res;
  for(unsigned long i = 0; i < N; i++){
    for(unsigned long j = 0; j < M; j++){
      res[j][i] = left[j][i]*right;
    }
  }
  return res;
}

namespace LavaCake {
  

  

  float Deg2Rad(float value);


  vec3f cross(vec3f const& left,
              vec3f const& right) ;

  vec3d cross(vec3d const& left,
              vec3d const& right) ;

  /*bool operator== (vec3f const& left,
                   vec3f const& right) ;

  mat2f inverse(const mat2f& m) ;

  mat4f inverse(mat4f& m) ;

  mat4f inverse(const mat4f& m) ;

  mat2d inverse(const mat2d& m) ;*/


  mat4f Identity() ;

  mat4f PrepareTranslationMatrix(float x,
                                float y,
                                float z) ;
  mat4f PrepareRotationMatrix(float           angle,
                             vec3f const & axis,
                             float           normalize_axis = 0.0f) ;

  mat4f PrepareScalingMatrix(float x,
                            float y,
                            float z) ;

  mat4f PreparePerspectiveProjectionMatrix(float aspect_ratio,
                                          float field_of_view,
                                          float near_plane,
                                          float far_plane) ;

  mat4f PrepareOrthographicProjectionMatrix(float left_plane,
                                           float right_plane,
                                           float bottom_plane,
                                           float top_plane,
                                           float near_plane,
                                           float far_plane) ;

}

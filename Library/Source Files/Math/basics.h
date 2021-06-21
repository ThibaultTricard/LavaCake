#pragma once

#include <tuple>

namespace LavaCake {

  using vec2f = std::array<float, 2>;
  using vec2d = std::array<double, 2>;
  using vec2i = std::array<int, 2>;
  using vec2u = std::array<uint32_t, 2>;
  using vec2b = std::array<bool, 2>;

  using vec3f = std::array<float, 3>;
  using vec3d = std::array<double, 3>;
  using vec3i = std::array<int, 3>;
  using vec3u = std::array<uint32_t, 3>;
  using vec3b = std::array<bool, 3>;

  using vec4f = std::array<float, 4>;
  using vec4d = std::array<double, 4>;
  using vec4i = std::array<int, 4>;
  using vec4u = std::array<uint32_t, 4>;
  using vec4b = std::array<bool, 4>;

  using mat2 = std::array<float, 4>;
  using mat3 = std::array<float, 9>;
  using mat4 = std::array<float, 16>;


  template<typename T, unsigned long N>
  std::array<T, N> operator*(const std::array<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator*(const T a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a;
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator*(const std::array<T, N>& a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] * a[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator/(const std::array<T, N>& a, const std::array<T, N>& b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] / b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator/(const std::array<T, N>& b, const T a) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = b[i] / a;
    }
    return std::array<T, N>(d);
  }


  template<typename T, unsigned long N>
  std::array<T, N> operator-(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] - b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  std::array<T, N> operator+(const std::array<T, N> a, const std::array<T, N> b) {
    std::array<T, N> d = std::array<T, N>();
    for (unsigned char i = 0; i < N; i++) {
      d[i] = a[i] + b[i];
    }
    return std::array<T, N>(d);
  }

  template<typename T, unsigned long N>
  T dot(std::array<T, N> const & left,
        std::array<T, N> const & right ){
    T sum = static_cast<T>(0);
    for(unsigned long u = 0; u< N ; u++){
      sum +=  left[u] * right[u];
    }
    return sum;
  };

  template<typename T, unsigned long N>
  std::array<T, N> normalize(std::array<T, N> const vector) {
    float length = dot(vector,vector);
    if (length >0)
      return vector/length;
    return vector;
  }

  float Deg2Rad(float value) {
    return value * 0.01745329251994329576923690768489f;
  }

  float Dot(vec3f const& left,
            vec3f const& right) {
    return dot(left,right);
  }

  vec3f cross(vec3f const& left,
              vec3f const& right) {
    return  vec3f({
      left[1] * right[2] - left[2] * right[1],
      left[2] * right[0] - left[0] * right[2],
      left[0] * right[1] - left[1] * right[0]
    });
  }
  vec3f Cross(vec3f const& left,
              vec3f const& right) {
    return  cross(left, right);
  }

  vec3f Normalize(vec3f const& vector) {
    return normalize(vector);
  }


  vec3f operator* (vec3f const& left,
                   mat4 const& right) {
    return vec3f({
      left[0] * right[0] + left[1] * right[1] + left[2] * right[2],
      left[0] * right[4] + left[1] * right[5] + left[2] * right[6],
      left[0] * right[8] + left[1] * right[9] + left[2] * right[10]
    });
  }

  bool operator== (vec3f const& left,
                   vec3f const& right) {
    if ((std::abs(left[0] - right[0]) > 0.00001f) ||
        (std::abs(left[1] - right[1]) > 0.00001f) ||
        (std::abs(left[2] - right[2]) > 0.00001f)) {
      return false;
    }
    else {
      return true;
    }
  }

  mat4 operator* (mat4 const& left,
                  mat4 const& right) {
    return mat4({
      left[0] * right[0] + left[4] * right[1] + left[8] * right[2] + left[12] * right[3],
      left[1] * right[0] + left[5] * right[1] + left[9] * right[2] + left[13] * right[3],
      left[2] * right[0] + left[6] * right[1] + left[10] * right[2] + left[14] * right[3],
      left[3] * right[0] + left[7] * right[1] + left[11] * right[2] + left[15] * right[3],
      
      left[0] * right[4] + left[4] * right[5] + left[8] * right[6] + left[12] * right[7],
      left[1] * right[4] + left[5] * right[5] + left[9] * right[6] + left[13] * right[7],
      left[2] * right[4] + left[6] * right[5] + left[10] * right[6] + left[14] * right[7],
      left[3] * right[4] + left[7] * right[5] + left[11] * right[6] + left[15] * right[7],
      
      left[0] * right[8] + left[4] * right[9] + left[8] * right[10] + left[12] * right[11],
      left[1] * right[8] + left[5] * right[9] + left[9] * right[10] + left[13] * right[11],
      left[2] * right[8] + left[6] * right[9] + left[10] * right[10] + left[14] * right[11],
      left[3] * right[8] + left[7] * right[9] + left[11] * right[10] + left[15] * right[11],
      
      left[0] * right[12] + left[4] * right[13] + left[8] * right[14] + left[12] * right[15],
      left[1] * right[12] + left[5] * right[13] + left[9] * right[14] + left[13] * right[15],
      left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15],
      left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15]
    });
  }


  mat4 inverse(mat4& m) {
    float inv[16], det;
    int i;
    
    mat4 invM;
    
    inv[0] = m[5] * m[10] * m[15] -
    m[5] * m[11] * m[14] -
    m[9] * m[6] * m[15] +
    m[9] * m[7] * m[14] +
    m[13] * m[6] * m[11] -
    m[13] * m[7] * m[10];
    
    inv[4] = -m[4] * m[10] * m[15] +
    m[4] * m[11] * m[14] +
    m[8] * m[6] * m[15] -
    m[8] * m[7] * m[14] -
    m[12] * m[6] * m[11] +
    m[12] * m[7] * m[10];
    
    inv[8] = m[4] * m[9] * m[15] -
    m[4] * m[11] * m[13] -
    m[8] * m[5] * m[15] +
    m[8] * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];
    
    inv[12] = -m[4] * m[9] * m[14] +
    m[4] * m[10] * m[13] +
    m[8] * m[5] * m[14] -
    m[8] * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];
    
    inv[1] = -m[1] * m[10] * m[15] +
    m[1] * m[11] * m[14] +
    m[9] * m[2] * m[15] -
    m[9] * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];
    
    inv[5] = m[0] * m[10] * m[15] -
    m[0] * m[11] * m[14] -
    m[8] * m[2] * m[15] +
    m[8] * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];
    
    inv[9] = -m[0] * m[9] * m[15] +
    m[0] * m[11] * m[13] +
    m[8] * m[1] * m[15] -
    m[8] * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];
    
    inv[13] = m[0] * m[9] * m[14] -
    m[0] * m[10] * m[13] -
    m[8] * m[1] * m[14] +
    m[8] * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];
    
    inv[2] = m[1] * m[6] * m[15] -
    m[1] * m[7] * m[14] -
    m[5] * m[2] * m[15] +
    m[5] * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];
    
    inv[6] = -m[0] * m[6] * m[15] +
    m[0] * m[7] * m[14] +
    m[4] * m[2] * m[15] -
    m[4] * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];
    
    inv[10] = m[0] * m[5] * m[15] -
    m[0] * m[7] * m[13] -
    m[4] * m[1] * m[15] +
    m[4] * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];
    
    inv[14] = -m[0] * m[5] * m[14] +
    m[0] * m[6] * m[13] +
    m[4] * m[1] * m[14] -
    m[4] * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];
    
    inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];
    
    inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];
    
    inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];
    
    inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];
    
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    
    if (det == 0)
      return invM;
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++)
      invM[i] = inv[i] * det;
    
    return invM;
    
  }

  mat4 inverse(const mat4& m) {
    float inv[16], det;
    int i;
    
    mat4 invM;
    
    inv[0] = m[5] * m[10] * m[15] -
    m[5] * m[11] * m[14] -
    m[9] * m[6] * m[15] +
    m[9] * m[7] * m[14] +
    m[13] * m[6] * m[11] -
    m[13] * m[7] * m[10];
    
    inv[4] = -m[4] * m[10] * m[15] +
    m[4] * m[11] * m[14] +
    m[8] * m[6] * m[15] -
    m[8] * m[7] * m[14] -
    m[12] * m[6] * m[11] +
    m[12] * m[7] * m[10];
    
    inv[8] = m[4] * m[9] * m[15] -
    m[4] * m[11] * m[13] -
    m[8] * m[5] * m[15] +
    m[8] * m[7] * m[13] +
    m[12] * m[5] * m[11] -
    m[12] * m[7] * m[9];
    
    inv[12] = -m[4] * m[9] * m[14] +
    m[4] * m[10] * m[13] +
    m[8] * m[5] * m[14] -
    m[8] * m[6] * m[13] -
    m[12] * m[5] * m[10] +
    m[12] * m[6] * m[9];
    
    inv[1] = -m[1] * m[10] * m[15] +
    m[1] * m[11] * m[14] +
    m[9] * m[2] * m[15] -
    m[9] * m[3] * m[14] -
    m[13] * m[2] * m[11] +
    m[13] * m[3] * m[10];
    
    inv[5] = m[0] * m[10] * m[15] -
    m[0] * m[11] * m[14] -
    m[8] * m[2] * m[15] +
    m[8] * m[3] * m[14] +
    m[12] * m[2] * m[11] -
    m[12] * m[3] * m[10];
    
    inv[9] = -m[0] * m[9] * m[15] +
    m[0] * m[11] * m[13] +
    m[8] * m[1] * m[15] -
    m[8] * m[3] * m[13] -
    m[12] * m[1] * m[11] +
    m[12] * m[3] * m[9];
    
    inv[13] = m[0] * m[9] * m[14] -
    m[0] * m[10] * m[13] -
    m[8] * m[1] * m[14] +
    m[8] * m[2] * m[13] +
    m[12] * m[1] * m[10] -
    m[12] * m[2] * m[9];
    
    inv[2] = m[1] * m[6] * m[15] -
    m[1] * m[7] * m[14] -
    m[5] * m[2] * m[15] +
    m[5] * m[3] * m[14] +
    m[13] * m[2] * m[7] -
    m[13] * m[3] * m[6];
    
    inv[6] = -m[0] * m[6] * m[15] +
    m[0] * m[7] * m[14] +
    m[4] * m[2] * m[15] -
    m[4] * m[3] * m[14] -
    m[12] * m[2] * m[7] +
    m[12] * m[3] * m[6];
    
    inv[10] = m[0] * m[5] * m[15] -
    m[0] * m[7] * m[13] -
    m[4] * m[1] * m[15] +
    m[4] * m[3] * m[13] +
    m[12] * m[1] * m[7] -
    m[12] * m[3] * m[5];
    
    inv[14] = -m[0] * m[5] * m[14] +
    m[0] * m[6] * m[13] +
    m[4] * m[1] * m[14] -
    m[4] * m[2] * m[13] -
    m[12] * m[1] * m[6] +
    m[12] * m[2] * m[5];
    
    inv[3] = -m[1] * m[6] * m[11] +
    m[1] * m[7] * m[10] +
    m[5] * m[2] * m[11] -
    m[5] * m[3] * m[10] -
    m[9] * m[2] * m[7] +
    m[9] * m[3] * m[6];
    
    inv[7] = m[0] * m[6] * m[11] -
    m[0] * m[7] * m[10] -
    m[4] * m[2] * m[11] +
    m[4] * m[3] * m[10] +
    m[8] * m[2] * m[7] -
    m[8] * m[3] * m[6];
    
    inv[11] = -m[0] * m[5] * m[11] +
    m[0] * m[7] * m[9] +
    m[4] * m[1] * m[11] -
    m[4] * m[3] * m[9] -
    m[8] * m[1] * m[7] +
    m[8] * m[3] * m[5];
    
    inv[15] = m[0] * m[5] * m[10] -
    m[0] * m[6] * m[9] -
    m[4] * m[1] * m[10] +
    m[4] * m[2] * m[9] +
    m[8] * m[1] * m[6] -
    m[8] * m[2] * m[5];
    
    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    
    if (det == 0)
      return invM;
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++)
      invM[i] = inv[i] * det;
    
    return invM;
    
  }



mat4 Identity() {
  mat4 I = mat4({
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
  });
  return I;
}

mat4 PrepareTranslationMatrix(float x,
                              float y,
                              float z) {
  mat4 translation_matrix = mat4({
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    x,    y,    z, 1.0f
  });
  return translation_matrix;
}

mat4 PrepareRotationMatrix(float           angle,
                           vec3f const & axis,
                           float           normalize_axis) {
  float x;
  float y;
  float z;
  
  if (normalize_axis) {
    vec3f normalized = Normalize(axis);
    x = normalized[0];
    y = normalized[1];
    z = normalized[2];
  }
  else {
    x = axis[0];
    y = axis[1];
    z = axis[2];
  }
  
  const float c = cos(Deg2Rad(angle));
  const float _1_c = 1.0f - c;
  const float s = sin(Deg2Rad(angle));
  
  mat4 rotation_matrix = mat4({
    x * x * _1_c + c,
    y * x * _1_c - z * s,
    z * x * _1_c + y * s,
    0.0f,
    
    x * y * _1_c + z * s,
    y * y * _1_c + c,
    z * y * _1_c - x * s,
    0.0f,
    
    x * z * _1_c - y * s,
    y * z * _1_c + x * s,
    z * z * _1_c + c,
    0.0f,
    
    0.0f,
    0.0f,
    0.0f,
    1.0f
  });
  return rotation_matrix;
}

mat4 PrepareScalingMatrix(float x,
                          float y,
                          float z) {
  mat4 scaling_matrix = mat4({
    x, 0.0f, 0.0f, 0.0f,
    0.0f,    y, 0.0f, 0.0f,
    0.0f, 0.0f,    z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  });
  return scaling_matrix;
}

mat4 PreparePerspectiveProjectionMatrix(float aspect_ratio,
                                        float field_of_view,
                                        float near_plane,
                                        float far_plane) {
  float f = 1.0f / tan(Deg2Rad(0.5f * field_of_view));
  
  mat4 perspective_projection_matrix = mat4({
    f / aspect_ratio,
    0.0f,
    0.0f,
    0.0f,
    
    0.0f,
    -f,
    0.0f,
    0.0f,
    
    0.0f,
    0.0f,
    far_plane / (near_plane - far_plane),
    -1.0f,
    
    0.0f,
    0.0f,
    (near_plane * far_plane) / (near_plane - far_plane),
    0.0f
  });
  return perspective_projection_matrix;
}

mat4 PrepareOrthographicProjectionMatrix(float left_plane,
                                         float right_plane,
                                         float bottom_plane,
                                         float top_plane,
                                         float near_plane,
                                         float far_plane) {
  mat4 orthographic_projection_matrix = mat4({
    2.0f / (right_plane - left_plane),
    0.0f,
    0.0f,
    0.0f,
    
    0.0f,
    2.0f / (bottom_plane - top_plane),
    0.0f,
    0.0f,
    
    0.0f,
    0.0f,
    1.0f / (near_plane - far_plane),
    0.0f,
    
    -(right_plane + left_plane) / (right_plane - left_plane),
    -(bottom_plane + top_plane) / (bottom_plane - top_plane),
    near_plane / (near_plane - far_plane),
    1.0f
  });
  return orthographic_projection_matrix;
}

}

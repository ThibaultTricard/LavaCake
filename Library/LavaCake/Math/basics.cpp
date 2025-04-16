#include "basics.h"

namespace LavaCake {

float Deg2Rad(float value) {
  return value * 0.01745329251994329576923690768489f;
}

vec3f cross(vec3f const& left,
            vec3f const& right) {
  return  vec3f({
    left[1] * right[2] - left[2] * right[1],
    left[2] * right[0] - left[0] * right[2],
    left[0] * right[1] - left[1] * right[0]
  });
}

vec3d cross(vec3d const& left,
            vec3d const& right) {
  return  vec3d({
    left[1] * right[2] - left[2] * right[1],
    left[2] * right[0] - left[0] * right[2],
    left[0] * right[1] - left[1] * right[0]
  });
}



mat4f Identity() {
  mat4f I = mat4f({
    1.0f,0.0f,0.0f,0.0f,
    0.0f,1.0f,0.0f,0.0f,
    0.0f,0.0f,1.0f,0.0f,
    0.0f,0.0f,0.0f,1.0f
  });
  return I;
}

mat4f PrepareTranslationMatrix(float x,
                              float y,
                              float z) {
  mat4f translation_matrix = mat4f({
    1.0f, 0.0f, 0.0f, x,
    0.0f, 1.0f, 0.0f, y,
    0.0f, 0.0f, 1.0f, z,
    0.0f,  0.0f,   0.0f, 1.0f
  });
  return translation_matrix;
}

mat4f PrepareRotationMatrix(float           angle,
                           vec3f const & axis,
                           float           normalize_axis) {
  float x;
  float y;
  float z;
  
  if (normalize_axis) {
    vec3f normalized = normalize(axis);
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
  
  mat4f rotation_matrix = mat4f({
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

mat4f PrepareScalingMatrix(float x,
                          float y,
                          float z) {
  mat4f scaling_matrix = mat4f({
    x, 0.0f, 0.0f, 0.0f,
    0.0f,    y, 0.0f, 0.0f,
    0.0f, 0.0f,    z, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  });
  return scaling_matrix;
}

mat4f PreparePerspectiveProjectionMatrix(float aspect_ratio,
                                        float field_of_view,
                                        float near_plane,
                                        float far_plane) {
  float f = 1.0f / tan(Deg2Rad(0.5f * field_of_view));
  
  mat4f perspective_projection_matrix = mat4f({
    f / aspect_ratio,
    0.0f,
    0.0f,
    0.0f,
    
    0.0f,
    f,
    0.0f,
    0.0f,
    
    0.0f,
    0.0f,
    (far_plane)/  ( far_plane - near_plane),
    -(far_plane * near_plane)/  ( far_plane - near_plane) ,
    
    0.0f,
    0.0f,
    1.0f,
    0.0f
  });


  return perspective_projection_matrix;
}

mat4f PrepareOrthographicProjectionMatrix(float left_plane,
                                         float right_plane,
                                         float bottom_plane,
                                         float top_plane,
                                         float near_plane,
                                         float far_plane) {
  mat4f orthographic_projection_matrix = mat4f({
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

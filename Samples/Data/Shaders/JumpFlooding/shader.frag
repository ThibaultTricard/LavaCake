#version 450

layout( set = 0, binding = 0 ,rgba32f) uniform imageBuffer result;
layout( set = 0, binding = 1 ) uniform UniformBuffer {
  ivec2 dim;
  int pass;
};

layout( location = 0 ) in vec2 texcoord;

layout( location = 0 ) out vec4 frag_color;

void main() {
  vec2 coord = texcoord * vec2(dim);
  vec4 r = imageLoad(result,int(int(coord.x) + int(coord.y) * dim.x)) ;
  vec4 color;
  if(r.w < 10.0){
    color = vec4(0.0,1.0,0.0,1.0);
  }
  else{
    color = vec4(r.z);
  }
  frag_color = color;
}

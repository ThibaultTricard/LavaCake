#version 450

layout( set = 0, binding = 0 ,r32f) uniform imageBuffer transform;
layout( set = 0, binding = 1 ) uniform UniformBuffer {
  uint sizeX;
  uint sizeY;
};

layout( location = 0 ) in vec2 texcoord;

layout( location = 0 ) out vec4 frag_color;

void main() {
  vec2 coord = texcoord - vec2(0.5);
  if (coord.x < 0.0){
  	coord.x+= 1.0;
  }
  if (coord.y < 0.0){
  	coord.y+= 1.0;
  }
  coord = coord  * vec2(sizeX, sizeY) ;
  float r = imageLoad(transform,int(int(coord.x) + int(coord.y) * sizeX)).x ;
  float i = imageLoad(transform,int(int(coord.x) + int(coord.y) * sizeX + sizeX* sizeY)).x;
  frag_color = vec4(sqrt(r*r+i*i)/1000.0);
}

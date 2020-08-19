#version 450
layout( set = 0, binding = 0 ) uniform sampler2D framebuffer;
layout( location = 1 ) in vec2 frag_uv;
layout( location = 0 ) out vec4 frag_color;

float dot[] = {0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,1,1,0,0,0,
			   0,0,0,1,1,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,0,0};

float sla[] = {0,0,0,0,0,0,0,0,
			   0,0,0,0,0,0,1,0,
			   0,0,0,0,0,1,1,0,
			   0,0,0,0,1,1,0,0,
			   0,0,0,1,1,0,0,0,
			   0,0,1,1,0,0,0,0,
			   0,1,1,0,0,0,0,0,
			   0,0,0,0,0,0,0,0};

float plu[] = {0,0,0,0,0,0,0,0,
			   0,0,0,1,1,0,0,0,
			   0,0,0,1,1,0,0,0,
			   0,1,1,1,1,1,1,0,
			   0,1,1,1,1,1,1,0,
			   0,0,0,1,1,0,0,0,
			   0,0,0,1,1,0,0,0,
			   0,0,0,0,0,0,0,0};

float ash[] = {0,0,0,0,0,0,0,0,
			   0,1,1,0,0,1,1,0,
			   0,1,1,0,0,1,1,0,
			   0,1,1,1,1,1,1,0,
			   0,1,1,1,1,1,1,0,
			   0,1,1,0,0,1,1,0,
			   0,1,1,0,0,1,1,0,
			   0,0,0,0,0,0,0,0};

void main() {

  vec2 uv = fract(frag_uv * 64.0);
  vec3 color = texture(framebuffer,frag_uv- uv/64.0).xyz;

  if(color.x<0.2){
  	color = vec3(0.0);
  }
  else if(color.x < 0.4){
  	ivec2 c = ivec2(uv * 8.0);
  	color = vec3(dot[c.x + c.y*8]);
  }
  else if(color.x < 0.6){
	ivec2 c = ivec2(uv * 8.0);
  	color = vec3(sla[c.x + c.y*8]);
  }
  else if(color.x < 0.8){
	ivec2 c = ivec2(uv * 8.0);
  	color = vec3(plu[c.x + c.y*8]);
  }
  else if(color.x < 0.95) {
  	ivec2 c = ivec2(uv * 8.0);
  	color = vec3(ash[c.x + c.y*8]);
  }
  else{
  	color = vec3(1.0);

  }
  frag_color = vec4(color, 0.0);
}

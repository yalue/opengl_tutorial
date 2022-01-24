#version 330 core

in VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

layout(std140) uniform Lighting {
  // These are all vec3's, but padded to vec4
  vec4 position;
  vec4 color;
  vec4 ambient_color;
  float ambient_power;
  float pad[3];
} lighting;

void main() {
  frag_color = vec4(lighting.color.xyz, 1.0);
}


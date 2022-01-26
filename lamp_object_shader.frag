#version 330 core

in VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

layout(std140) uniform SharedUniforms {
  mat4 projection;
  mat4 view;
  // Use vec4's rather than vec3's for alignment reasons.
  vec4 lamp_position;
  vec4 lamp_color;
  vec4 ambient_color;
  float ambient_power;
  float pad[3];
} shared_uniforms;

void main() {
  frag_color = vec4(shared_uniforms.lamp_color.xyz, 1.0);
}


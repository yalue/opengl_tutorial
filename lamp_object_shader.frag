#version 330 core

in VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

// Replaced with shared_uniforms.glsl in our code.
//INCLUDE_SHARED_UNIFORMS

void main() {
  frag_color = vec4(shared_uniforms.lamp_color.xyz, 1.0);
}


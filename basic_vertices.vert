#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 texture_coord_in;
layout (location = 3) in mat4 model_transform_in;
layout (location = 7) in mat3 normal_transform_in;

out VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} vs_out;

// Replaced with shared_uniforms.glsl in our code.
//INCLUDE_SHARED_UNIFORMS

void main() {
  gl_Position = shared_uniforms.projection * shared_uniforms.view *
    model_transform_in * vec4(position_in, 1.0);
  vs_out.texture_coord = texture_coord_in;
  vs_out.normal = normal_in * normal_transform_in;
  // Fragment position, for lighting computations.
  vs_out.frag_position = vec3(model_transform_in * vec4(position_in, 1.0));
}


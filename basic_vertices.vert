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
  gl_Position = shared_uniforms.projection * shared_uniforms.view *
    model_transform_in * vec4(position_in, 1.0);
  vs_out.texture_coord = texture_coord_in;
  vs_out.normal = normal_in * normal_transform_in;
  // Fragment position, for lighting computations.
  vs_out.frag_position = vec3(model_transform_in * vec4(position_in, 1.0));
}

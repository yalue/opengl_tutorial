#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 texture_coord_in;
layout (location = 3) in mat4 model_transform_in;
layout (location = 7) in mat3 normal_transform_in;

out vec2 texture_coord;
out vec3 normal;
out vec3 frag_position;

uniform mat4 projection_transform;
uniform mat4 view_transform;

void main() {
  gl_Position = projection_transform * view_transform * model_transform_in *
    vec4(position_in, 1.0);
  texture_coord = texture_coord_in;
  normal = normal_in * normal_transform_in;
  // Fragment position, for lighting computations.
  frag_position = vec3(model_transform_in * vec4(position_in, 1.0));
}

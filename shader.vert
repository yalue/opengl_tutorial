#version 330 core

layout (location = 0) in vec3 position_in;
layout (location = 1) in vec3 color_in;
layout (location = 2) in vec2 texture_coord_in;

out vec3 vertex_color;
out vec2 texture_coord;

uniform mat4 transform;

void main() {
  gl_Position = transform * vec4(position_in, 1.0);
  vertex_color = color_in;
  texture_coord = texture_coord_in;
}

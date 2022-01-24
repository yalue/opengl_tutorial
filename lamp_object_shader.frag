#version 330 core

in vec2 texture_coord;
in vec3 normal;
in vec3 frag_position;
out vec4 frag_color;

struct Lighting {
  vec3 position;
  vec3 color;
  vec3 ambient_color;
  float ambient_power;
};

uniform Lighting lighting;

void main() {
  frag_color = vec4(lighting.color, 1.0);
}


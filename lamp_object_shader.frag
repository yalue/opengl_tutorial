#version 330 core

in VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} fs_in;

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


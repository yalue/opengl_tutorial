#version 330 core

in vec2 texture_coord;
in vec3 normal;
in vec3 frag_position;
out vec4 frag_color;

uniform sampler2D texture0;
uniform sampler2D texture1;

struct Lighting {
  vec3 position;
  vec3 color;
  vec3 ambient_color;
  float ambient_power;
};

uniform Lighting lighting;

void main() {
  vec4 tex_color = mix(texture(texture0, texture_coord), texture(texture1,
    texture_coord), 0.2);
  float alpha = tex_color.z;
  vec3 ambient_light = lighting.ambient_color * lighting.ambient_power;
  vec3 n_normal = normalize(normal);
  vec3 light_dir = normalize(lighting.position - frag_position);
  float diffuse_power = max(dot(n_normal, light_dir), 0.0);
  vec3 diffuse_color = lighting.color * diffuse_power;
  frag_color = vec4(vec3(tex_color) * (ambient_light + diffuse_color), alpha);
}


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

uniform sampler2D texture0;

void main() {
  vec4 tex_color = texture(texture0, fs_in.texture_coord);
  float alpha = tex_color.z;
  vec3 ambient_light = lighting.ambient_color.xyz * lighting.ambient_power;
  vec3 normal = normalize(fs_in.normal);
  vec3 light_dir = normalize(lighting.position.xyz - fs_in.frag_position);
  float diffuse_power = max(dot(normal, light_dir), 0.0);
  vec3 diffuse_color = lighting.color.xyz * diffuse_power;
  frag_color = vec4(vec3(tex_color) * (ambient_light + diffuse_color), alpha);
}


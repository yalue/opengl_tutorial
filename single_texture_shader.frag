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

uniform sampler2D texture0;

void main() {
  vec4 tex_color = texture(texture0, fs_in.texture_coord);
  float alpha = tex_color.z;
  vec3 ambient_light = shared_uniforms.ambient_color.xyz *
    shared_uniforms.ambient_power;
  vec3 normal = normalize(fs_in.normal);
  vec3 light_dir = normalize(shared_uniforms.lamp_position.xyz -
    fs_in.frag_position);
  float diffuse_power = max(dot(normal, light_dir), 0.0);
  vec3 diffuse_color = shared_uniforms.lamp_color.xyz * diffuse_power;
  frag_color = vec4(vec3(tex_color) * (ambient_light + diffuse_color), alpha);
}


#version 330 core

in VS_OUT {
  vec2 texture_coord;
  vec3 normal;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

// Replaced with shared_uniforms.glsl in our code.
//INCLUDE_SHARED_UNIFORMS

uniform sampler2D texture0;

void main() {
  vec4 tex_color = texture(texture0, fs_in.texture_coord);
  float alpha = tex_color.z;
  vec3 ambient_light = shared_uniforms.ambient_color.xyz *
    shared_uniforms.ambient_power;
  vec3 normal = normalize(fs_in.normal);
  vec3 light_dir = normalize(shared_uniforms.lamp_position.xyz -
    fs_in.frag_position);

  // Diffuse component
  float diffuse_power = max(dot(normal, light_dir), 0.0);
  vec3 diffuse_color = shared_uniforms.lamp_color.xyz * diffuse_power;

  // Specular component
  float specular_scale = 0.5;
  vec3 view_dir = normalize(shared_uniforms.view_position.xyz -
    fs_in.frag_position);
  vec3 reflect_dir = reflect(-light_dir, normal);
  float specular_power = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
  vec3 specular_color = specular_scale * specular_power *
    shared_uniforms.lamp_color.xyz;

  vec3 final_light = ambient_light + diffuse_color + specular_color;
  frag_color = vec4(vec3(tex_color) * final_light, alpha);
}


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
uniform sampler2D texture1;

void main() {
  vec4 tex_color = mix(texture(texture0, fs_in.texture_coord),
    texture(texture1, fs_in.texture_coord), 0.2);
  float alpha = tex_color.z;
  vec3 ambient_light = shared_uniforms.ambient_color.xyz *
    shared_uniforms.ambient_power;
  vec3 normal = normalize(fs_in.normal);
  vec3 light_dir = normalize(shared_uniforms.lamp_position.xyz -
    fs_in.frag_position);

  // Diffuse component
  float diffuse_power = max(dot(normal, light_dir), 0.0);
  vec3 diffuse_color = shared_uniforms.lamp_color.xyz * diffuse_power;

  vec3 specular_color = vec3(0, 0, 0);
  // Specular component
  float specular_scale = 0.5;
  vec3 view_dir = normalize(shared_uniforms.view_position.xyz -
    fs_in.frag_position);
  vec3 reflect_dir = reflect(-light_dir, normal);
  float specular_power = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
  specular_color = specular_scale * specular_power *
    shared_uniforms.lamp_color.xyz;

  // Lamp power
  float lamp_dist = length(shared_uniforms.lamp_position.xyz -
    fs_in.frag_position);
  float attenuation = 1.0 / (shared_uniforms.lamp_constant +
    shared_uniforms.lamp_linear * lamp_dist +
    shared_uniforms.lamp_quadratic * lamp_dist * lamp_dist);

  // NOTE: could also attenuate ambient here.
  vec3 final_light = ambient_light + attenuation * (diffuse_color +
    specular_color);
  frag_color = vec4(vec3(tex_color) * final_light, alpha);
}


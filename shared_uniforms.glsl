layout(std140) uniform SharedUniforms {
  mat4 projection;
  mat4 view;
  // Use vec4's rather than vec3's for alignment reasons.
  vec4 lamp_position;
  vec4 lamp_color;
  vec4 ambient_color;
  vec4 view_position;
  float ambient_power;
  float lamp_constant;
  float lamp_linear;
  float lamp_quadratic;
} shared_uniforms;


#version 330 core

in vec2 texture_coord;
out vec4 frag_color;

uniform sampler2D box_texture;
uniform sampler2D face_texture;

void main() {
  frag_color = mix(texture(box_texture, texture_coord), texture(face_texture,
    texture_coord), 0.2);
}


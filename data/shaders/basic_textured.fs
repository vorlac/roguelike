#version 330 core

in vec2 tex_coord;
in vec4 vcolor;

out vec4 out_color;

uniform sampler2D sprite;

void main() {
 
  out_color = texture(sprite, tex_coord) * vcolor;
  // out_color = vec4(1.0, 1.0, 1.0, 1.0);
  // out_color = vec4(tex_coord.x, tex_coord.y, 1.0, 1.0);
}

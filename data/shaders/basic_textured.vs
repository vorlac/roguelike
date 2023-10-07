#version 330 core

in vec2 position;
in vec2 uv;
in vec4 color;

out vec2 tex_coord;
out vec4 vcolor;

uniform mat4 ortho;

void main() {
  tex_coord = uv;
  vcolor = color;
  gl_Position = ortho * vec4(position, 0, 1);
}
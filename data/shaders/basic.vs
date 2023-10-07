#version 330 core

in vec2 position;
in vec4 color;

out vec4 vcolor;

uniform mat4 ortho;

void main() {
  vcolor = color;
  gl_Position = ortho * vec4(position, 0, 1);
}
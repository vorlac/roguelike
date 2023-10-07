#version 330 core

in vec2 position;
in vec2 uv;
in float tile;
in vec4 fg_color;
in vec4 bg_color;

out vec2 tex_coord;
out vec4 fg;
out vec4 bg;

uniform mat4 ortho;
uniform vec2 offset;

void main() {
  float inc = 1.0 / 16.0;
  int t = int(tile);
  int tile_x = (t % 16);
  int tile_y = (t / 16);
  tex_coord = uv + vec2(tile_x*inc, (tile_y*inc));
  fg = fg_color;
  bg = bg_color;
  vec2 newPosition = position + offset;
  gl_Position = (ortho * vec4(newPosition, 0, 1));
}

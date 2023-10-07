#version 330 core

in vec2 tex_coord;
in vec4 fg;
in vec4 bg;

out vec4 out_color;

uniform sampler2D sprite;

void main() {
  vec4 color = texture(sprite, tex_coord);
  if (color == vec4(1.0, 0.0, 1.0, 1.0) || color.a == 0.0) {
    color = bg;
  }
  else {
    vec4 tint = fg;
    float r = min(color[0], tint[0]);
    float g = min(color[1], tint[1]);
    float b = min(color[2], tint[2]);
    float a = color[3];
    color = vec4(r, g, b, a);
  }
  out_color = color;
}

/*
uniform samplerBuffer u_color_data;

struct ColorData {
  vec4 fg;
  vec4 bg;
};

ColorData fetch_color_data(uint index) {
  ColorData value = {};
  value.fg = texelFetch(u_color_data, index/2);
  value.bg = texelFetch(u_color_data, index/2+1);
  return value;



  ------------------------------------------------------


  attribute vec2 a_position;
attribute vec2 a_uv;
attribute int a_index;

struct Character {
  vec4 fg;
  vec4 bg;
  int tile;
};

layout(std140, binding=0) uniform CharacterData {
  Character u_charater_data[];
};

varying vec4 v_fg;

void main() {
  // ...
  v_fg = u_character_data[a_index].fg;
  // ...
}
}*/

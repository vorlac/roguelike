#version 330 core

in vec4 primitive_color;
out vec4 fragment_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragment_color = primitive_color;
}
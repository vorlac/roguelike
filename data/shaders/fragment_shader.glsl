#version 330 core

in vec4 primitive_color;
out vec4 fragment_color;

void main()
{
    fragment_color = primitive_color;
}
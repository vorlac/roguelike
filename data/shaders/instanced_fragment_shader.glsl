#version 330 core

out vec4 fragment_color;  
in vec4 primitive_color;
  
void main()
{
    fragment_color = primitive_color;
}
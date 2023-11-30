#version 330 core

out vec4 fragment_color;  
in vec3 primitive_color;
  
void main()
{
    fragment_color = vec4(primitive_color, 1.0);
}
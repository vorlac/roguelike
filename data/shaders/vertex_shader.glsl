#version 330 core

layout (location = 0) in vec3 pos;   
layout (location = 1) in vec4 in_color; 

out vec4 primitive_color; 
uniform mat4 transform;  

void main()
{
    gl_Position = transform * vec4(pos, 1.0f);
    // set ourColor to the input color we got from the vertex data
    primitive_color = in_color; 
}
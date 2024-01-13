#version 330 core

layout (location = 0) in vec3 pos;   
layout (location = 1) in vec4 in_color; 
layout (location = 2) in vec3 offset;   

uniform mat4 mvp;

// output a color to the fragment shader
out vec4 primitive_color; 

void main()
{
	gl_Position = mvp * vec4(pos + offset, 1.0f);
    primitive_color = in_color;
}
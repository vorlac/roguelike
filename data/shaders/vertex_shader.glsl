#version 330 core
layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    //gl_Position = vec4(aPos, 1.0);

	// w = 1 -> point 
	// w = 0 -> vector
	gl_Position = projection * view * model * vec4(pos, 1);
}
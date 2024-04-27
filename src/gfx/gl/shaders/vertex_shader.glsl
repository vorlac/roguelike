#version 330 core

// the position variable has attribute position 0
layout (location = 0) in vec3 pos;   
// the color variable has attribute position 1
layout (location = 1) in vec4 in_color; 
 

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// output a color to the fragment shader
out vec4 primitive_color; 

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0f);
    primitive_color = in_color; //vec4(0.831372559f, 0.643137276f, 0.643137276f, 1.0f); // in_color;
}
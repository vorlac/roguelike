#version 330 core
 
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec4 color;

// temp, just been using for testing, 
vec3 velocity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 primitive_color;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);
    primitive_color = color;
}
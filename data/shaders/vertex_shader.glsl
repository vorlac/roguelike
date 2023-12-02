#version 330 core

layout(location = 1) in vec3 pos;
layout(location = 2) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 primitive_color;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);
    // set ourColor to the input color we got from the vertex data
    primitive_color = color;
}

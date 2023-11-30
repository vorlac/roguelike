#version 330 core
// the position variable has attribute position 0
layout (location = 0) in vec3 pos;   
// the color variable has attribute position 1
layout (location = 1) in vec3 inColor; 
// output a color to the fragment shader
out vec3 primitive_color; 

void main()
{
    gl_Position = vec4(pos, 1.0);
    // set ourColor to the input color we got from the vertex data
    primitive_color = inColor; 
}
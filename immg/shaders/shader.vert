#version 450 core

layout (location = 0) in vec3 attr_position;
layout (location = 1) in vec3 attr_color;

uniform mat4 projection;

out vec3 vertex_color;

void main()
{
    gl_Position = projection * vec4(attr_position, 1.0);
    vertex_color = attr_color;
}

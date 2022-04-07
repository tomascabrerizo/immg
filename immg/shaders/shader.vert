#version 450 core

layout (location = 0) in vec2 attr_position;
layout (location = 1) in vec2 attr_uvs;
layout (location = 2) in vec3 attr_color;

uniform mat4 projection;

out vec3 vertex_color;
out vec2 vertex_uvs;

void main()
{
    gl_Position = projection * vec4(attr_position, 0.0, 1.0);
    vertex_color = attr_color;
    vertex_uvs = attr_uvs;
}

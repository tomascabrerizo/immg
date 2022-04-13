#version 450 core

in vec3 vertex_color;
in vec2 vertex_uvs;
out vec4 color;

uniform sampler2D sampler_texture;

void main()
{
    color = vec4(1.0, 1.0, 1.0, texture(sampler_texture, vertex_uvs).r);
    //color = texture(sampler_texture, vertex_uvs);
}

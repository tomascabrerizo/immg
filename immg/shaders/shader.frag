#version 450 core

in vec3 vertex_color;
in vec2 vertex_uvs;
out vec4 color;

uniform sampler2D sampler_texture;

void main()
{
    vec4 scolor = texture(sampler_texture, vertex_uvs);
    color = vec4(vec3(scolor.r), 1);
}

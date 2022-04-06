#include <SDL.h>
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "imm_math.h"

void *imm_read_entire_file(const char *path, u64 *file_size)
{
    FILE *file = fopen(path, "rb"); 
    fseek(file, 0, SEEK_END);
    *file_size = (u64)ftell(file);
    fseek(file, 0, SEEK_SET);
    void *buffer = malloc(*file_size + 1);
    fread(buffer, (size_t)*file_size, 1, file);
    ((char *)buffer)[*file_size] = 0;
    fclose(file);
    return buffer;
}

struct imm_texture_t
{
    void *pixels;
    u32 width, height;
};

imm_texture_t imm_load_bmp(const char *path)
{
    u64 file_size;
    void *file = imm_read_entire_file(path, &file_size);
    
    u8 *header = (u8 *)file;
    u32 pixel_offset = *(u32 *)(header + 10);
    u8 *info_header = header + 14;
    u32 width = *(u32 *)(info_header + 4);
    u32 height = *(u32 *)(info_header + 8);
    u16 bpp = *(u16 *)(info_header + 14);
    assert(bpp == 32);
    
    u64 texture_size = width * height * 4;
    imm_texture_t texture = {};
    texture.pixels = malloc(texture_size);
    memcpy(texture.pixels, (void *)(header + pixel_offset), texture_size);
    texture.width = width;
    texture.height = height;

    free(file);

    return texture;
}

unsigned int imm_load_gl_shader(const char *vertex, const char *fragment)
{
    u64 vertex_size, fragment_size;
    void *vertex_file = imm_read_entire_file(vertex, &vertex_size);
    void *fragment_file = imm_read_entire_file(fragment, &fragment_size);
    
    int vertex_compile, fragment_compile, program_link; 
    
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &(char *)vertex_file, 0);
    glCompileShader(vertex_shader);
    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compile);
    if(vertex_compile != GL_TRUE)
    {
        u64 log_length = 0;
        char message[1024];
        glGetShaderInfoLog(vertex_shader, 1024, (GLsizei *)&log_length, message);
        printf("[vertex-shader-error]:\n%s\n", message);
    }

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &(char *)fragment_file, 0);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compile);
    if(fragment_compile != GL_TRUE)
    {
        u64 log_length = 0;
        char message[1024];
        glGetShaderInfoLog(fragment_shader, 1024, (GLsizei *)&log_length, message);
        printf("[fragment-shader-error]:\n%s\n", message);
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
     
    glGetProgramiv(program, GL_LINK_STATUS, &program_link);
    if(program_link != GL_TRUE)
    {
        u64 log_length = 0;
        char message[1024];
        glGetProgramInfoLog(program, 1024, (GLsizei *)&log_length, message);
        printf("[program-link-error]:\n%s\n", message);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    free(vertex_file);
    free(fragment_file);

    return program;
}

// TODO: vertex should have texture coordinates, propably the gui should be able to render
// texture and not texture rects for memory optimisations
struct imm_vertex_t
{
    float x, y;
    float r, g, b;
};

// TODO: make gui struct to handle all state in one place
// NOTE: internal gui state

#define imm_index_offset(a) (imm_index_offset + (a))

static imm_vertex_t imm_vertex_buffer[256];
static u32 imm_vertex_buffer_size = 256;
static u32 imm_vertex_buffer_count = 0;

static u32 imm_index_buffer[256];
static u32 imm_index_buffer_size = 256;
static u32 imm_index_buffer_count = 0;
static u32 imm_index_offset = 0;

void imm_render_push_rect(s32 x, s32 y, s32 width, s32 height, f32 red, f32 green, f32 blue)
{
    f32 min_x = (f32)x;
    f32 min_y = (f32)y;
    f32 max_x = (f32)(x + width  - 1);
    f32 max_y = (f32)(y + height - 1);
    
    imm_vertex_t r[4];
    r[0] = {min_x, min_y, red, green, blue};
    r[1] = {min_x, max_y, red, green, blue};
    r[2] = {max_x, max_y, red, green, blue};
    r[3] = {max_x, min_y, red, green, blue};
    memcpy(imm_vertex_buffer + imm_vertex_buffer_count, r, sizeof(r));
    imm_vertex_buffer_count += 4;
    
    u32 i[6] = 
    { 
        imm_index_offset(0), imm_index_offset(1), imm_index_offset(3),
        imm_index_offset(1), imm_index_offset(2), imm_index_offset(3)
    };
    memcpy(imm_index_buffer + imm_index_buffer_count, i, sizeof(i));
    imm_index_buffer_count += 6;
    imm_index_offset += 4;
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    int window_width = 800;
    int window_height = 600;
    SDL_Window *window = SDL_CreateWindow("immg", 
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                          window_width, window_height, SDL_WINDOW_OPENGL);

    int error = 0;
    error += SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    error += SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    if(error)
    {
        printf("[gl-error]: %s\n", SDL_GetError());
    }

    SDL_GLContext gl_ctx = SDL_GL_CreateContext(window);
    
    if(!gladLoadGL())
    {
        printf("[gl-error]: error initiallising GLAD\n");
    }

    unsigned int vao, vbo, ibo;

    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glCreateBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(imm_vertex_buffer), (void *)imm_vertex_buffer, GL_DYNAMIC_DRAW);
    // TODO: create offset off macro to make this code more readable and less error prone
    glEnableVertexAttribArray(0); // NOTE: vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void *)(sizeof(float)*0));
    glEnableVertexAttribArray(1); // NOTE: vretex colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void *)(sizeof(float)*2));

    glCreateBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(imm_index_buffer), (void *)imm_index_buffer, GL_DYNAMIC_DRAW);
    
    unsigned int shader = imm_load_gl_shader("shaders/shader.vert", "shaders/shader.frag");
    glUseProgram(shader);
    
    m4 projection = m4_ortho(0, (f32)window_width, 0, (f32)window_height, 0, 1.0f);
    int projection_loc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, (const float *)projection.m);
    
    // TODO: load texture
    // TODO: bind the texture that each element (only have rects for now) want to use 
    // TODO: be able to copy array of texture in the GPU

    bool running = true;
    while(running)
    {
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_QUIT:
            {
                running = false;
            }break;   
            }
        }
        
        imm_render_push_rect(100, 100, 200, 200, 1.0f, 0.0f, 0.0f); 
        imm_render_push_rect(400, 100, 200, 100, 0.0f, 1.0f, 0.0f); 
        imm_render_push_rect(10, 10, 20, 20, 0.0f, 1.0f, 1.0f); 

        // TODO: test if glBufferSubData us faster than glMapBuffer
        // NOTE: copy gui buffers into GPU 
        void *vertex_buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(vertex_buffer, imm_vertex_buffer, imm_vertex_buffer_count * sizeof(imm_vertex_t));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        void *index_buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(index_buffer, imm_index_buffer, imm_index_buffer_count * sizeof(u32));
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawElements(GL_TRIANGLES, imm_index_buffer_count, GL_UNSIGNED_INT, 0);
        SDL_GL_SwapWindow(window);

        // NOTE: clear gui buffers
        imm_vertex_buffer_count = 0;
        imm_index_buffer_count = 0;
        imm_index_offset = 0;
    }

    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);

    return 0;
}

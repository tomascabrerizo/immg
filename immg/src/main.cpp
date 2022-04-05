#include <SDL.h>
#include <glad/glad.h>
#include <stdio.h>

#include "imm_math.h"

static float rect[12] =
{
    0.0f, 0.0f, 0.0f, 
    0.0f, 1.0f, 0.0f, 
    1.0f, 1.0f, 0.0f, 
    1.0f, 0.0f, 0.0f, 
};

static int indices[6] = 
{
    0, 1, 3,
    1, 2, 3
};

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect), (void *)rect, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glCreateBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), (void *)indices, GL_STATIC_DRAW);
    
    unsigned int shader = imm_load_gl_shader("shaders/shader.vert", "shaders/shader.frag");
    glUseProgram(shader);
    
    m4 projection = m4_ortho(0, (f32)window_width, 0, (f32)window_height, 0, 1.0f);
    int projection_loc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, (const float *)projection.m);
    
    m4 model = m4_translate(_v3(100, 200, 0)) * m4_scale(_v3(200, 100, 0));
    int model_loc = glGetUniformLocation(shader, "model");
    glUniformMatrix4fv(model_loc, 1, GL_TRUE, (const float *)model.m);

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
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(gl_ctx);
    SDL_DestroyWindow(window);

    return 0;
}

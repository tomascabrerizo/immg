#include <SDL.h>
#include <glad/glad.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H 

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "imm_math.h"

struct imm_character_t
{
    v2 min_uv;
    v2 max_uv;
    v2 size;
    v2 baring;
    int advance;
};

// NOTE: simple hash table to save the text rendering metrics
// NOTE: for simplicity is a static hash table and use internal probing

struct imm_character_hash_bucket_t
{
    bool used;
    char glyph;
    imm_character_t metric;
};

#define imm_character_hash_size 512
struct imm_character_hash_t
{
    imm_character_hash_bucket_t data[imm_character_hash_size];    
};

void imm_character_hash_add(imm_character_hash_t *hash, char glyph, imm_character_t metric)
{
    u32 index = glyph & (imm_character_hash_size - 1);
    imm_character_hash_bucket_t *bucket = hash->data + index;
    while(bucket->used)
    {
        index++;
        assert(index < imm_character_hash_size);
        bucket = hash->data + index;
    }
    bucket->used = true;
    bucket->glyph = glyph;
    bucket->metric = metric;
}

imm_character_t *imm_character_hash_get(imm_character_hash_t *hash, char glyph)
{
    u32 index = glyph & (imm_character_hash_size - 1);
    imm_character_hash_bucket_t *bucket = hash->data + index;
    if(bucket->used)
    {
        while(bucket->glyph != glyph)
        {
            index++;
            assert(index < imm_character_hash_size);
            bucket = hash->data + index;
        }
        return &bucket->metric;
    }
    return 0;
}

struct imm_character_atlas_t
{
    unsigned int texture_id;
    imm_character_hash_t hash;
    char *buffer;
    u32 width;
    u32 height;
    u32 font_size;
};

enum imm_character_atlas_type_t
{
    character_atlas_type_small,
    character_atlas_type_large,
    
    character_atlas_type_count,
};

static imm_character_atlas_t character_atlas[character_atlas_type_count];

void imm_character_atlas_init(imm_character_atlas_t *atlas, const char *path, u32 width, u32 height, u32 font_size, u32 padding)
{
    FT_Library ft;
    if(FT_Init_FreeType(&ft))
    {
        printf("[freetype-error]: could not init free type library\n");
    }
    FT_Face face; 
    if(FT_New_Face(ft, path, 0, &face))
    {
        printf("[freetype-error]: fail to load font\n");
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    u32 atlas_height = height; 
    u32 atlas_width = width;

    char *atlas_buffer = (char *)malloc(atlas_width * atlas_height);
    memset(atlas_buffer, 0, (atlas_width * atlas_height));
    
    u32 atlas_x_offset = padding;
    u32 atlas_y_offset = padding;
    
    for(u8 c = ' '; c < 128; ++c)
    {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("[freetype-error]: fail to load glyph %c\n", c);
            continue;
        }

        u32 glyph_width = face->glyph->bitmap.width;
        u32 glyph_height = face->glyph->bitmap.rows;
        s32 glyph_pitch = face->glyph->bitmap.pitch;
        
        if((atlas_x_offset + glyph_width) >= atlas_width)
        {
            atlas_x_offset = padding;
            atlas_y_offset += font_size + padding; 
        }

        for(u32 y = 0; y < glyph_height; ++y)
        {
            for(u32 x = 0; x < glyph_width; ++x)
            {
                u32 y_pos = y + atlas_y_offset;
                u32 x_pos = x + atlas_x_offset;
                // NOTE: Clipping the atlas to the width and height
                // TODO: this can be optimize, now is really slow
                if((y_pos < atlas_height) && (x_pos < atlas_width))
                {
                    atlas_buffer[(y_pos * atlas_width) + x_pos] = face->glyph->bitmap.buffer[y * glyph_pitch + x];
                }
            }
        }
        
        f32 u_0 = ((f32)(atlas_x_offset) / (f32)atlas_width);
        f32 v_0 = ((f32)(atlas_y_offset) / (f32)atlas_height);
        f32 u_1 = ((f32)(atlas_x_offset + glyph_width) / (f32)atlas_width);
        f32 v_1 = ((f32)(atlas_y_offset + glyph_height) / (f32)atlas_height);

        atlas_x_offset += glyph_width + padding;

        imm_character_t character =
        {
            _v2(u_0, v_0), // NOTE: texture_coord_0
            _v2(u_1, v_1), // NOTE: texture_coord_1
            _v2((f32)glyph_width, (f32)glyph_height),
            _v2((f32)face->glyph->bitmap_left, (f32)face->glyph->bitmap_top),
            face->glyph->advance.x 
        };
        imm_character_hash_add(&atlas->hash, c, character);
        atlas->width = atlas_width;
        atlas->height = atlas_height;
        atlas->buffer = atlas_buffer;
        atlas->font_size = font_size;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void imm_character_atlas_init_types()
{
    imm_character_atlas_init(&character_atlas[character_atlas_type_small], "data/bitstream_vera_sans/Vera.ttf", 256, 256, 16, 4);
    imm_character_atlas_init(&character_atlas[character_atlas_type_large], "data/bitstream_vera_sans/Vera.ttf", 256, 256, 24, 4);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glCreateTextures(GL_TEXTURE_2D, 1, &character_atlas[character_atlas_type_small].texture_id);
    glBindTexture(GL_TEXTURE_2D, character_atlas[character_atlas_type_small].texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, character_atlas[character_atlas_type_small].width, character_atlas[character_atlas_type_small].height, 0, GL_RED, GL_UNSIGNED_BYTE, character_atlas[character_atlas_type_small].buffer);
    glGenerateMipmap(GL_TEXTURE_2D);

    glCreateTextures(GL_TEXTURE_2D, 1, &character_atlas[character_atlas_type_large].texture_id);
    glBindTexture(GL_TEXTURE_2D, character_atlas[character_atlas_type_large].texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, character_atlas[character_atlas_type_large].width, character_atlas[character_atlas_type_large].height, 0, GL_RED, GL_UNSIGNED_BYTE, character_atlas[character_atlas_type_large].buffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
}

void imm_character_atlas_write_to_disk(imm_character_atlas_t *atlas, const char *path)
{
    if(atlas)
    {
        stbi_write_bmp(path, atlas->width, atlas->height, 1, (void *)atlas->buffer);
    }
}

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
    s32 pitch;
};

imm_texture_t imm_texture_load_bmp(const char *path)
{
    u64 file_size;
    void *file = imm_read_entire_file(path, &file_size);
    
    u8 *header = (u8 *)file;
    u32 pixel_offset = *(u32 *)(header + 10);
    u8 *info_header = header + 14;
    u32 width = *(u32 *)(info_header + 4);
    u32 height = *(u32 *)(info_header + 8);
    u16 bpp = *(u16 *)(info_header + 14);
    
    u64 texture_size = width * height * (bpp / 8);
    imm_texture_t texture = {};
    texture.pixels = malloc(texture_size);
    memcpy(texture.pixels, (void *)(header + pixel_offset), texture_size);
    texture.width = width;
    texture.height = height;
    texture.pitch = width * (bpp / 8);

    free(file);

    return texture;
}

void imm_texture_free(imm_texture_t *texture)
{
    if(texture)
    {
        free(texture->pixels);
        texture->pixels = 0;
    }
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
    float u, v;
    float r, g, b;
};

// TODO: make gui struct to handle all state in one place
// NOTE: internal gui state

#define imm_index_offset(a) (imm_index_offset + (a))

static imm_vertex_t imm_vertex_buffer[1024];
static u32 imm_vertex_buffer_size = 1024;
static u32 imm_vertex_buffer_count = 0;

static u32 imm_index_buffer[1024];
static u32 imm_index_buffer_size = 1024;
static u32 imm_index_buffer_count = 0;
static u32 imm_index_offset = 0;

void imm_render_push_rect_raw(v2 pos, v2 dim, v3 color, v2 min_uv, v2 max_uv)
{
    f32 min_x = pos.x;
    f32 min_y = pos.y;
    f32 max_x = (pos.x + dim.x);
    f32 max_y = (pos.y + dim.y);
    
    imm_vertex_t r[4];
    r[0] = {min_x, min_y, min_uv.x, min_uv.y, color.x, color.y, color.z};
    r[1] = {min_x, max_y, min_uv.x, max_uv.y, color.x, color.y, color.z};
    r[2] = {max_x, max_y, max_uv.x, max_uv.y, color.x, color.y, color.z};
    r[3] = {max_x, min_y, max_uv.x, min_uv.y, color.x, color.y, color.z};
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

void imm_render_push_text_rect(s32 x, s32 y, char *text, imm_character_atlas_type_t type)
{
    glBindTexture(GL_TEXTURE_2D, character_atlas[type].texture_id);

    u32 base = character_atlas[type].font_size;
    for(char *c = text; *c != '\0'; ++c)
    {
        imm_character_t *character = imm_character_hash_get(&character_atlas[type].hash, *c);
        
        v2 position = {};
        position.x = x + character->baring.x;
        position.y = y + (base - character->baring.y);

        imm_render_push_rect_raw(position, character->size, _v3(0, 0, 0), character->min_uv, character->max_uv);

        x += (u32)((character->advance >> 6));
    }
}

void imm_render_push_rect(s32 x, s32 y, s32 width, s32 height, f32 red, f32 green, f32 blue)
{
    imm_render_push_rect_raw(_v2((f32)x, (f32)y), _v2((f32)width, (f32)height), _v3((f32)red, (f32)green, (f32)blue), _v2(0, 0), _v2(0, 0));
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    int window_width = 1024;
    int window_height = 512;
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void *)(sizeof(float)*0));
    glEnableVertexAttribArray(1); // NOTE: vertex uv coordinates 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void *)(sizeof(float)*2));
    glEnableVertexAttribArray(2); // NOTE: vretex colors
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(imm_vertex_t), (const void *)(sizeof(float)*4));

    glCreateBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(imm_index_buffer), (void *)imm_index_buffer, GL_DYNAMIC_DRAW);
    
    unsigned int shader = imm_load_gl_shader("shaders/shader.vert", "shaders/shader.frag");
    glUseProgram(shader);
    
    m4 projection = m4_ortho(0, (f32)window_width, 0, (f32)window_height, 0, 1.0f);
    int projection_loc = glGetUniformLocation(shader, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, (const float *)projection.m);
    
    // NOTE: load font test
    imm_character_atlas_init_types();
    imm_character_atlas_write_to_disk(&character_atlas[character_atlas_type_small], "data/character_atlas_small.bmp");
    imm_character_atlas_write_to_disk(&character_atlas[character_atlas_type_large], "data/character_atlas_large.bmp");

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
        
        imm_render_push_text_rect(20, 100, "Tomas Cabrerizo!", character_atlas_type_large); 
        imm_render_push_text_rect(20, 200, "Gonzalo Cabrerizo!", character_atlas_type_large); 
        imm_render_push_text_rect(20, 250, "Manuel Cabrerizo!", character_atlas_type_large); 

        // TODO: test if glBufferSubData us faster than glMapBuffer
        // NOTE: copy gui buffers into GPU 
        void *vertex_buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(vertex_buffer, imm_vertex_buffer, imm_vertex_buffer_count * sizeof(imm_vertex_t));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        void *index_buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(index_buffer, imm_index_buffer, imm_index_buffer_count * sizeof(u32));
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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

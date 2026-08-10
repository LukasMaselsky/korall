#ifndef KORALL_GL_H
#define KORALL_GL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "utils/utils.h"

#define MAX_SHADER_SOURCE_LEN (4 * KILOBYTE)

typedef struct {
	float r, g, b, a;
} Colour;

#define COLOUR(r_c, g_c, b_c, a_c) { .r = r_c, .g = g_c, .b = b_c, .a = a_c }

extern const Colour COLOUR_BG;


//

void gl_framebuffer_size_callback(GLFWwindow* window, int width, int height);

GLFWwindow* gl_window_create(int width, int height, const char* title);

GLFWwindow* gl_init(int width, int height, const char* title);

void gl_cleanup();

//

int gl_shader_load(const char* name, char* o_shader, size_t len);

GLuint gl_shader_compile(GLenum shader_type, const GLchar* source);

void gl_shader_set_int(GLuint shader_id, const char* name, int value);

void gl_shader_set_bool(GLuint shader_id, const char* name, bool value);

void gl_shader_set_float(GLuint shader_id, const char* name, float value);

GLuint gl_shaders_link(GLuint* shaders, size_t shader_count);

void gl_shaders_use(GLuint shader_program);

//

void gl_vbo_bind(GLenum mode, GLuint vbo);

void gl_vbo_unbind(GLenum mode);

void gl_vao_bind(GLuint vao);

void gl_vao_unbind();

void gl_vbo_set(GLenum mode, GLuint vbo, GLsizeiptr size, const GLvoid* data, GLenum usage);

void gl_vao_set(
	GLuint index, 
	GLint size, 
	GLenum type, 
	GLboolean normalised, 
	GLuint stride_num, 
	GLsizei data_point_size, 
	GLuint offset
);

void gl_vao_set_float(GLuint index, GLint size, GLuint stride_num, GLuint offset);

//

void gl_draw_array(GLuint vao, GLenum mode, GLint start_index, GLsizei count);


#endif
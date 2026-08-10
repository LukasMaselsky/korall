#include "gl.h"

const Colour COLOUR_BG = COLOUR(0.2f, 0.3f, 0.3f, 1.0f);

void gl_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

GLFWwindow* gl_window_create(int width, int height, const char* title) {
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
	{
		KORALL_LOG(LOG_ERR, "Failed to create GLFW window\n");
		glfwTerminate();
		return NULL;
	}
	return window;
}

GLFWwindow* gl_init(int width, int height, const char *title) {

	if (glfwInit() == GLFW_FALSE) return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = gl_window_create(width, height, title);
	if (window == NULL) return NULL;
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{

		KORALL_LOG(LOG_ERR, "Failed to initialize GLAD\n");
		return NULL;
	}

	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, gl_framebuffer_size_callback);
	return window;
}

void gl_cleanup() {
	glfwTerminate();
}

// SHADERS

int gl_shader_load(const char *name, char *o_shader, size_t len) {
	const char path[MAX_FILE_PATH + 1] = { 0 };
	const char* base = KORALL_RESOURCES_PATH "shaders/";
	int res = snprintf(path, MAX_FILE_PATH, "%s%s", base, name);
	if (res != strlen(base) + strlen(name)) {
		KORALL_LOG(LOG_ERR, "Couldn't load shader, filename \"%s\" too long\n", name);
		return -1;
	}
	FILE* f = fopen(path, "rb");
	if (f == NULL)
	{
		KORALL_LOG(LOG_ERR, "Couldn't load shader \"%s\"\n", name);
		return -1;
	};
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	if (fsize > len) {
		KORALL_LOG(LOG_ERR, "Couldn't load shader \"%s\", file too large\n", name);
		return -1;
	}
	fread(o_shader, fsize, 1, f);
	fclose(f);
}

/**
 * @brief creates and compiles a shader
 * @param shader_type
 * @param source glsl code
 * @return
 */
GLuint gl_shader_compile(GLenum shader_type, const GLchar* source) {
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		KORALL_LOG(LOG_ERR, "shader compilation failed: %s\n", infoLog);
	}
	return shader;
}

void gl_shader_set_int(GLuint shader_id, const char* name, int value) {
	glUniform1i(glGetUniformLocation(shader_id, name), (int)value);
}

void gl_shader_set_bool(GLuint shader_id, const char *name, bool value) {
	gl_shader_set_int(shader_id, name, (int)value);
}

void gl_shader_set_float(GLuint shader_id, const char* name, float value) {
	glUniform1f(glGetUniformLocation(shader_id, name), value);
}

/**
 * @brief create a shader program
 * @param shaders
 * @param shader_count
 * @return shader program
 */
GLuint gl_shaders_link(GLuint* shaders, size_t shader_count) {
	GLuint shader_program = glCreateProgram();
	for (int i = 0; i < shader_count; i++) {
		glAttachShader(shader_program, shaders[i]);
	}
	glLinkProgram(shader_program);

	int success;
	char infoLog[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
		KORALL_LOG(LOG_ERR, "shader linking failed: %s\n", infoLog);
	}
	for (int i = 0; i < shader_count; i++) {
		glDeleteShader(shaders[i]);
	}
	return shader_program;
}

void gl_shaders_use(GLuint shader_program) {
	glUseProgram(shader_program);
}

// https://stackoverflow.com/a/33494201

void gl_vbo_bind(GLenum mode, GLuint vbo) {
	glBindBuffer(mode, vbo);
}

void gl_vbo_unbind(GLenum mode) {
	gl_vbo_bind(mode, 0);
}

void gl_vao_bind(GLuint vao) {
	glBindVertexArray(vao);
}

void gl_vao_unbind() {
	gl_vao_bind(0);
}

void gl_vbo_set(GLenum mode, GLuint vbo, GLsizeiptr size, const GLvoid* data, GLenum usage) {
	glBindBuffer(mode, vbo);
	glBufferData(mode, size, data, usage);
}

/**
 * @brief
 * @param index
 * @param size of vertex attr (1,2,3,4)
 * @param type
 * @param normalised
 * @param stride_num
 * @param data_point_size sizeof(data_type)
 * @param offset
 */
void gl_vao_set(GLuint index, GLint size, GLenum type, GLboolean normalised, GLuint stride_num, GLsizei data_point_size, GLuint offset) {
	glVertexAttribPointer(index, size, type, normalised, stride_num * data_point_size, (void*)(offset * data_point_size));
}

void gl_vao_set_float(GLuint index, GLint size, GLuint stride_num, GLuint offset) {
	gl_vao_set(index, size, GL_FLOAT, GL_FALSE, stride_num, sizeof(GLfloat), offset);
}

void gl_draw_array(GLuint vao, GLenum mode, GLint start_index, GLsizei count) {
	gl_vao_bind(vao);
	glDrawArrays(mode, start_index, count);
	gl_vao_unbind();
}

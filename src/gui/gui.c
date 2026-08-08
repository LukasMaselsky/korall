#include "gui.h"

const char* vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
	"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
	"}\n\0";

float vertices[] = {
		-0.5f, -0.5f, 0.0f, // left  
		 0.5f, -0.5f, 0.0f, // right 
		 0.0f,  0.5f, 0.0f  // top   
};


static void gui_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

static GLFWwindow *gui_window_create(int width, int height, const char *title) {
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return NULL;
	}
	return window;
}

static GLFWwindow* gui_init() {

	if (glfwInit() == GLFW_FALSE) return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	int width = 800;
	int height = 600;
	const char* title = "korall";

	GLFWwindow* window = gui_window_create(width, height, title);
	if (window == NULL) return NULL;
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return NULL;
	}

	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, gui_framebuffer_size_callback);
	return window;
}

static void gui_cleanup() {
	glfwTerminate();
}

//

/**
 * @brief creates and compiles a shader
 * @param shader_type 
 * @param source glsl code
 * @return 
 */
static GLuint gui_shader_compile(GLenum shader_type, const GLchar* source) {
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

/**
 * @brief create a shader program
 * @param shaders 
 * @param shader_count 
 * @return shader program
 */
static GLuint gui_shaders_link(GLuint *shaders, size_t shader_count) {
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

// https://stackoverflow.com/a/33494201

static inline void gui_vbo_bind(GLenum mode, GLuint vbo) {
	glBindBuffer(mode, vbo);
}

static inline void gui_vbo_unbind(GLenum mode) {
	gui_vbo_bind(mode, 0);
}

static inline void gui_vao_bind(GLuint vao) {
	glBindVertexArray(vao);
}

static inline void gui_vao_unbind() {
	gui_vao_bind(0);
}

static inline void gui_vbo_set(GLenum mode, GLuint vbo, GLsizeiptr size, const GLvoid* data, GLenum usage) {
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
static inline void gui_vao_set(GLuint index, GLint size, GLenum type, GLboolean normalised, GLuint stride_num, GLsizei data_point_size, GLuint offset) {
	glVertexAttribPointer(index, size, type, normalised, stride_num * data_point_size, (void*)(offset * data_point_size));
}

static inline void gui_vao_set_float(GLuint index, GLint size, GLuint stride_num, GLuint offset) {
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride_num * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
}

static inline void gui_draw_array(GLuint vao, GLenum mode, GLint start_index, GLsizei count) {
	gui_vao_bind(vao); 
	glDrawArrays(mode, start_index, count);
	gui_vao_unbind(); 
}

static void gui_setup(GLuint *program_out, GLuint *vao_out) {

	GLuint vertex_shader = gui_shader_compile(GL_VERTEX_SHADER, vertexShaderSource);

	GLuint fragment_shader = gui_shader_compile(GL_FRAGMENT_SHADER, fragmentShaderSource);

	GLuint shaders[2] = { vertex_shader, fragment_shader };
	GLuint shader_program = gui_shaders_link(shaders, 2);
	*program_out = shader_program;


	GLuint vbo, vao;
	glGenVertexArrays(1, &vao);
	*vao_out = vao;
	glGenBuffers(1, &vbo);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(vbo);

	gui_vbo_set(GL_ARRAY_BUFFER, vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

	gui_vao_set_float(0, 3, 3, 0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	gui_vbo_unbind(GL_ARRAY_BUFFER);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	gui_vao_unbind();
}

// opengl loop

static void gui_process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

static void gui_screen_clear() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}



/**
 * @brief render this every frame
 * @param shader_program 
 * @param vao 
 */
static void gui_render(GLuint shader_program, GLuint vao) {
	gui_screen_clear(); // ! important to have first

	glUseProgram(shader_program);
	gui_draw_array(vao, GL_TRIANGLES, 0, 3);
}


static void gui_main()
{
	const char* queue_name = GUI_MSG_QUEUE_NAME;

	mqd_t mq = msg_queue_open(queue_name, MSG_Q_READ);
	if (mq == INVALID_MQD) return;

	GLFWwindow* window = gui_init();
	if (window == NULL) return;

	GLuint shader_program, vao;
	gui_setup(&shader_program, &vao);


	Message msg = { 0 };
	while (!glfwWindowShouldClose(window))
	{
		// queue

		memset(&msg, 0, sizeof(msg));
		if (msg_queue_read(queue_name, &msg) == -1)
			continue;

		// input

		gui_process_input(window);

		// render

		gui_render(shader_program, vao);

		// 

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	

	msg_queue_close(mq);
	msg_queue_unlink(queue_name);
	gui_cleanup();
	return;
}

/**
 * @brief
 * @param thread [out] opaque type, could be struct
 * @param thread_id [out] id of created thread
 */
void gui_run(THREAD_T *thread, unsigned int *thread_id)
{
	if (thread_create(thread, gui_main, NULL, thread_id) == -1)
	{
		KORALL_LOG(LOG_ERR, "could not create GUI, exiting\n");
		exit(EXIT_FAILURE);
	}
}
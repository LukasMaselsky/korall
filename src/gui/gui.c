#include "gui.h"

float vertices[] = {
	// positions         // colors
	 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
	 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 

};

static GLuint gui_setup_shaders() {

	char vertexShaderSource[MAX_SHADER_SOURCE_LEN + 1] = { 0 };
	char fragmentShaderSource[MAX_SHADER_SOURCE_LEN + 1] = { 0 };
	if (gl_shader_load("shader.vert", vertexShaderSource, MAX_SHADER_SOURCE_LEN) == -1) return;
	if (gl_shader_load("shader.frag", fragmentShaderSource, MAX_SHADER_SOURCE_LEN) == -1) return;

	GLuint vertex_shader = gl_shader_compile(GL_VERTEX_SHADER, vertexShaderSource);

	GLuint fragment_shader = gl_shader_compile(GL_FRAGMENT_SHADER, fragmentShaderSource);

	GLuint shaders[2] = { vertex_shader, fragment_shader };
	GLuint shader_program = gl_shaders_link(shaders, 2);
	return shader_program;
}

static GLuint gui_setup_vos() {
	GLuint vbo, vao;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(vbo);

	gl_vbo_set(GL_ARRAY_BUFFER, vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	GLuint index = 0;
	GLuint vertex_size = 3;
	GLuint stride = 6;
	GLuint offset = 0;
	gl_vao_set_float(index, vertex_size, stride, offset);
	glEnableVertexAttribArray(index);
	// colours attribute
	index = 1;
	offset = 3;
	gl_vao_set_float(index, vertex_size, stride, offset);
	glEnableVertexAttribArray(index);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	gl_vbo_unbind(GL_ARRAY_BUFFER);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	gl_vao_unbind();
	return vao;
}

static void gui_setup(GLuint *program_out, GLuint *vao_out) {

	*program_out = gui_setup_shaders();

	*vao_out = gui_setup_vos();
}

// opengl loop

static void gui_process_input(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

static void gui_screen_clear(const Colour *bg) {
	glClearColor(bg->r, bg->g, bg->b, bg->a);
	glClear(GL_COLOR_BUFFER_BIT);
}

/**
 * @brief render this every frame
 * @param shader_program 
 * @param vao 
 */
static void gui_render(GLuint shader_program, GLuint vao) {
	gui_screen_clear(&COLOUR_BG); // ! important to have first

	gl_shaders_use(shader_program);
	gl_draw_array(vao, GL_TRIANGLES, 0, 3);
}


static void gui_main()
{
	const char* queue_name = GUI_MSG_QUEUE_NAME;

	mqd_t mq = msg_queue_open(queue_name, MSG_Q_READ);
	if (mq == INVALID_MQD) return;

	int width = 800;
	int height = 600;
	const char* title = "korall";
	GLFWwindow* window = gl_init(width, height, title);
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
	gl_cleanup();
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
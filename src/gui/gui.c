#include "gui.h"

static void gui_main()
{
	const char* queue_name = GUI_MSG_QUEUE_NAME;

	mqd_t mq = msg_queue_open(queue_name, MSG_Q_READ);
	if (mq == INVALID_MQD) return;

	while (true)
	{
		Message msg = {0};
		if (msg_queue_read(queue_name, &msg) == -1)
			continue;


		// KORALL_LOG(LOG_INFO, "main opengl stuff done here\n");
	}

	msg_queue_close(mq);
	msg_queue_unlink(queue_name);
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

static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int func()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
#ifndef KORALL_GUI_H
#define KORALL_GUI_H

#include <stdlib.h>
#include "utils/utils.h"
#include "gl/gl.h"
#include "thread/thread.h"
#include "msg_queue/msg_queue.h"

#ifdef _WIN32
#define GUI_MSG_QUEUE_NAME NULL
#else
#define GUI_MSG_QUEUE_NAME "/gui_queue"
#endif

void gui_run(THREAD_T* thread, unsigned int* thread_id);

#endif
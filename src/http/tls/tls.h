#ifndef KORALL_TLS_H
#define KORALL_TLS_H

#include "utils/logging/logging.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX* openssl_create_server_ctx(const char* path);

void openssl_init();

void openssl_cleanup();

#endif
#include "tls.h"
#include "utils/utils.h"

// https://www.techbuddies.io/2026/01/02/how-to-build-a-c-openssl-tls-client-server-over-tcp-and-udp/

SSL_CTX* openssl_create_server_ctx(const char* path) {
    if (path == NULL) {
        log_msg(LOG_WARN, "path to ssl resources cannot be NULL\n");
        return NULL;
    }
    const SSL_METHOD* method;
    SSL_CTX* ctx;
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    const char* cert_filename = "server.crt";
    const char* key_filename = "server.key";

    char cert_path[MAX_FILE_PATH + 1] = { 0 };
    char key_path[MAX_FILE_PATH + 1] = { 0 };
    if (str_concat(path, cert_filename, cert_path, MAX_FILE_PATH) != 0) {
        log_msg(LOG_WARN, "couldn't load cert, path too long.\n");
        return NULL;
    }
    if (str_concat(path, key_filename, key_path, MAX_FILE_PATH) != 0) {
        log_msg(LOG_WARN, "couldn't load key, path too long.\n");
        return NULL;
    }

    /* Load certificate and private key created earlier */
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate public key\n");
        SSL_CTX_free(ctx);
        return NULL;
    }
    /* Basic security hardening: disable legacy protocols */
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    return ctx;
}

void openssl_init() {
    /* For OpenSSL 1.1.0+ this is mostly automatic, but these calls are safe */
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void openssl_cleanup() {
    EVP_cleanup();
}
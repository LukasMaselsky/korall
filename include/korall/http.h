#ifndef KORALL_HTTP__H
#define KORALL_HTTP__H

typedef struct HTTPRequestStartLineInternal HTTPRequestStartLine;
typedef struct HTTPHeaderHostInternal HTTPHeaderHost;
typedef struct HTTPRequestWebsocketInternal HTTPRequestWebsocket;
typedef struct HTTPRequestInternal HTTPRequest;

typedef struct HTTPResponseInternal HTTPResponse;

int korall_request_param_get(const HTTPRequest* req, const char* field, char* value, size_t value_len);

int korall_request_header_get(const HTTPRequest* req, const char* field, char* value, size_t value_len);

char* korall_request_body_get(const HTTPRequest* req);

int korall_response_start_set(HTTPResponse* res, const HTTPRequest* req, const int code);

int korall_response_body_set(HTTPResponse* res, const char* body);

int korall_response_header_set(HTTPResponse* res, const char* field, const char* value);

#endif
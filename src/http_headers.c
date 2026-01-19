#include "http_headers.h"
#include "utils.h"
#include "http_.h"
#include "sockets.h"

int process_http_host(const char* value, HTTPRequest* req) {

	if (*value == '\0') return -1;
	char temp[MAX_DOMAIN_NAME_LEN + 1];

	bool with_port = false;
	int len = MAX_DOMAIN_NAME_LEN;
	int i = 0;
	for (char c = *value; c != '\0' && i < len; c = *(+value)) {
		if (c == ':') {
			with_port = true;
			i++;
			break;
		};
		temp[i] = c;
		i++;
	}
	temp[i] = '\0';

	// process domain

	if (strcmp("localhost", temp) == 0) {
		strcpy(req->headers->host->domain, "localhost")
	}
	else if (*temp == '[') {
		// ipv6 -> "Host: [....]"
		temp[i - 1] = '\0'; // remove ]
		if (is_valid_ipv6(temp)) {
			strcpy(req->headers->host->domain, ++temp);
		}
	}
	else if (is_valid_ipv4(temp)) {
		strcpy(req->headers->host->domain, temp);
	}
	else {
		return -1;
	}
	
	if (!with_port) return 0;
	
	// process port

	char port_temp[MAX_PORT_NUM_CHAR_LEN + 1];
	len = MAX_PORT_NUM_CHAR_LEN;
	int j = 0;
	char c;
	while ((c = value[i]) != '\0') {
		if (j >= len || !is_digit(c)) return -1; // port too long or not digit
		port_temp[i] = c;
		i++;
		j++;
	}

	if (is_valid_port(port_temp)) { 
		strcpy(req->headers->host->port, port_temp);
		return 0 
	};


	return -1;
}
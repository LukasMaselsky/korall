#include "http_headers.h"
#include "utils.h"
#include "http_.h"
#include "lookup_tables.h"
#include "sockets.h"

/*
	Make sure domain:port or domain is valid format
*/
int http_domain_port(const char* value, char* domain, char* port, bool *with_port) {
	if (*value == '\0') return -1;
	char temp[MAX_DOMAIN_LEN + 1] = { 0 };

	*with_port = false;
	int len = MAX_DOMAIN_NAME_LEN;
	int i = 0;
	for (char c = *value; c != '\0' && i < len; c = *(++value)) {
		if (c == ':') {
			*with_port = true;
			break;
		};
		temp[i] = c;
		i++;
	}
	temp[i] = '\0';

	// process domain

	if (strcmp("localhost", temp) == 0) {
		strcpy(domain, "localhost");
	}
	else if (*temp == '[') {
		// ipv6 -> "Host: [....]"
		temp[i - 1] = '\0'; // remove ]
		const char* temp_p = temp;
		if (is_valid_ipv6(temp)) {
			strcpy(domain, ++temp_p);
		}
	}
	else if (is_valid_ipv4(temp)) {
		strcpy(domain, temp);
	}
	else {
		return -1;
	}

	if (!(*with_port)) return 0;

	// process port

	char port_temp[MAX_PORT_NUM_CHAR_LEN + 1];
	len = MAX_PORT_NUM_CHAR_LEN;
	value++; // skip ':'
	i = 0;
	char c;
	while ((c = value[i]) != '\0') {
		if (i >= len || !is_digit(c)) return -1; // port too long or not digit
		port_temp[i] = c;
		i++;
	}
	port_temp[i] = '\0';

	if (is_valid_port(port_temp)) {
		strcpy(port, port_temp);
		return 0;
	};


	return -1;
}

int http_process_host(const char* value, HTTPRequest* req) {

	char domain[MAX_DOMAIN_LEN + 1] = { 0 };
	char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	bool with_port = false; // needed since function is used elsewhere, here can throw away
	int res = http_domain_port(value, domain, port, &with_port);
	if (res == -1) return -1;

	strncpy(req->headers->host->domain, domain, MAX_DOMAIN_LEN);
	strncpy(req->headers->host->port, port, MAX_PORT_NUM_CHAR_LEN);
	return 0;
}

int http_process_accept(const char* value, HTTPRequest* req) {
	// todo: multiple types
	int res = lookup_str_int(value, http_media_type_lookup_table, HTTP_MEDIA_TYPE_TABLE_COUNT, false);
	if (res == -1) return -1;

	req->headers->accept = res;
	return 0;
}
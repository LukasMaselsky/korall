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

	char port_temp[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
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

int http_process_weighted_list(
	const char* value, 
	HTTPRequest* req, 
	LookupEntry *table, 
	int table_len, 
	char *field_arr, 
	int field_arr_len, 
	HTTPWeightedField* res_arr
) {
	int i = 0;
	for (char c = *value; ; c = *(++value)) {
		if (c == ',' || c == ';' || c == '\0') {
			field_arr[i] = '\0';
			int res = lookup_str_int(field_arr, table, table_len, false);
			if (res == -1) return -1;

			res_arr->field = res;
			res_arr->weight = 1.0;

			if (c == ';') {
				// get weight
				++value; // skip ;
				if (!(value[0] == 'q' && value[1] == '=' && value[2] == '0' && value[3] == '.')) return -1; // invalid weight format
				value += 4;

				char float_temp[64 + 1] = { 0 };
				float_temp[0] = '0';
				float_temp[1] = '.';
				int j = 2;
				for (c = *value; c != ',' && j < 64; c = *(++value)) {
					if (c == '\0') {
						--value; // for skip spaces
						break;
					}
					if (!(c <= '9' && c >= '0')) return -1;
					float_temp[j] = c;
					j++;
				}
				if (float_temp[j - 1] == '.') return -1;
				float_temp[j] = '\0';
				double val = strtod(float_temp, NULL);
				res_arr->weight = val;
			}

			res_arr++;

			do {
				value++;
			} while (*value == ' '); // skip spaces
			c = *value;
			if (c == '\0') return 0;
			i = 0;
			memset(field_arr, 0, field_arr_len);
		}
		field_arr[i] = c;
		i++;
	}
	return 0;
}

int http_process_accept(const char* value, HTTPRequest* req) {
	char temp[MAX_MEDIA_TYPE_CHAR_LEN + 1] = { 0 };
	HTTPWeightedField* res_arr = req->headers->accept;
	return http_process_weighted_list(value, req, http_media_type_lookup_table, HTTP_MEDIA_TYPE_TABLE_COUNT, temp, MAX_MEDIA_TYPE_CHAR_LEN + 1, res_arr);
}

int http_process_accept_encoding(const char* value, HTTPRequest* req) {
	char temp[MAX_ENCODING_CHAR_LEN + 1] = { 0 };
	HTTPWeightedField* res_arr = req->headers->accept_encoding;
	return http_process_weighted_list(value, req, http_encoding_lookup_table, HTTP_ENCODING_TABLE_COUNT, temp, MAX_ENCODING_CHAR_LEN + 1, res_arr);
}
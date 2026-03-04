#include "http_headers.h"
#include "utils.h"
#include "http_.h"
#include "lookup_tables.h"
#include "sockets.h"

/*
	Make sure domain:port or domain is valid format
*/
HTTPError http_domain_port(const char* value, char* domain, char* port, bool *with_port) {
	if (*value == '\0') return HTTP_BAD_DOMAIN_PORT;
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
			strcpy(domain, ++temp_p); // todo: test
		}
	}
	else if (is_valid_ipv4(temp)) {
		strcpy(domain, temp);
	}
	else {
		return HTTP_BAD_DOMAIN;
	}

	if (!(*with_port)) return HTTP_SUCCESS;

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
		return HTTP_SUCCESS;
	};


	return HTTP_BAD_PORT;
}

HTTPError http_process_host(const char* value, HTTPRequest* req) {

	char domain[MAX_DOMAIN_LEN + 1] = { 0 };
	char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	bool with_port = false; // needed since function is used elsewhere, here can throw away
	HTTPError res = http_domain_port(value, domain, port, &with_port);
	if (res != HTTP_SUCCESS) return res;

	strncpy(req->headers->host->domain, domain, MAX_DOMAIN_LEN);
	strncpy(req->headers->host->port, port, MAX_PORT_NUM_CHAR_LEN);
	return HTTP_SUCCESS;
}

static int http_process_weighted_list(
	const char* value, 
	HTTPRequest* req, 
	LookupEntry *table, 
	int table_len, 
	char *field_arr, 
	const int field_arr_len, 
	Array* res_arr
) {
	int i = 0;
	for (char c = *value; ; c = *(++value)) {
		if (c == ',' || c == ';' || c == '\0') {
			field_arr[i] = '\0';
			int res = lookup_str_int(field_arr, table, table_len, false);
			if (res == -1) return -1;

			double weight = 1.0;
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
				weight = val;
			}

			HTTPWeightedField wf = { .field = res, .weight = weight};
			array_add(res_arr, (void *) &wf);
			
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

HTTPError http_process_accept(const char* value, HTTPRequest* req) {
	char temp[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	Array* res_arr = req->headers->accept;
	int res = http_process_weighted_list(value, req, http_media_type_lookup_table, HTTP_MEDIA_TYPE_TABLE_COUNT, temp, MAX_MEDIA_TYPE_LEN + 1, res_arr);
	if (res == -1) return HTTP_BAD_ACCEPT;
	return HTTP_SUCCESS;
}

HTTPError http_process_accept_encoding(const char* value, HTTPRequest* req) {
	char temp[MAX_ENCODING_CHAR_LEN + 1] = { 0 };
	Array* res_arr = req->headers->accept_encoding;
	int res = http_process_weighted_list(value, req, http_encoding_lookup_table, HTTP_ENCODING_TABLE_COUNT, temp, MAX_ENCODING_CHAR_LEN + 1, res_arr);
	if (res == -1) return HTTP_BAD_ACCEPT_ENC;
	return HTTP_SUCCESS;
}

HTTPError http_process_content_length(const char* value, HTTPRequest* req) {
	int val;
	str_to_int_errno res = str_to_int(&val, value, 10);
	if (res != STR_TO_INT_SUCCESS) return HTTP_BAD_CONTENT_LENGTH;

	req->headers->content_length = val;
	return HTTP_SUCCESS;
}

HTTPError http_process_content_type(const char* value, HTTPRequest* req) {
	char type[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	int colon = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, ';');
	if (colon == -1) {
		int res = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, '\0');
		if (res == -1) return HTTP_BAD_CONTENT_TYPE;
	}

	int mt = lookup_str_int(type, http_media_type_lookup_table, HTTP_MEDIA_TYPE_TABLE_COUNT, false);
	if (mt == -1) return HTTP_BAD_CONTENT_TYPE;

	req->headers->content_type->media_type = mt;
	if (colon == -1) return HTTP_SUCCESS; // no boundary/charset

	// skip spaces
	value++;
	while (*value == ' ')
		value++;

	if (mt == HTTP_MT_MTP || mt == HTTP_MT_MTP_FORM_DATA) {
		// boundary
		const char* pre = "boundary=";
		const int pre_len = 10;
		if (!starts_with(pre, value)) return HTTP_BAD_CONTENT_TYPE;
		value += (pre_len - 1);

		int boundary_res = fill_string_char(&value, req->headers->content_type->boundary, MAX_HTTP_BOUNDARY_LEN, '\0');
		if (boundary_res == -1) return HTTP_BAD_CONTENT_TYPE;
		return HTTP_SUCCESS;
	}
	else {
		// charset
		const char* pre = "charset=";
		const int pre_len = 9;
		if (!starts_with(pre, value)) return HTTP_BAD_CONTENT_TYPE;
		value += (pre_len - 1);

		char charset[MAX_HTTP_CHARSET_LEN + 1] = { 0 };
		int charset_res = fill_string_char(&value, charset, MAX_HTTP_CHARSET_LEN, '\0');
		if (charset_res == -1) return HTTP_BAD_CONTENT_TYPE;

		int charset_int = lookup_str_int(charset, http_charset_lookup_table, HTTP_CHARSET_TABLE_COUNT, true);
		if (charset_int == -1) return HTTP_BAD_CONTENT_TYPE;
		req->headers->content_type->charset = charset_int;
		return HTTP_SUCCESS;
	}
}

HTTPError http_process_access_control_request_method(const char* value, HTTPRequest* req) {
	int res = lookup_str_int(value, http_method_lookup_table, HTTP_METHOD_TABLE_COUNT, true);
	if (res == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_METHOD;
	req->headers->access_control_request_method = res;

	return HTTP_SUCCESS;
}

HTTPError http_process_access_control_request_headers(const char* value, HTTPRequest* req) {
	char val[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	HTTPRequestHeaderField field;
	while (fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, ',') != -1) {
		if ((field = lookup_str_int(val, http_req_header_field_lookup_table, HTTP_REQ_HEADER_FIELD_TABLE_COUNT, true)) == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
		if (!array_add(req->headers->access_control_request_headers, &field)) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, MAX_HTTP_HEADER_FIELD_LEN + 1);
	};

	if (fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, '\0') == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
	if ((field = lookup_str_int(val, http_req_header_field_lookup_table, HTTP_REQ_HEADER_FIELD_TABLE_COUNT, true)) == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
	if (!array_add(req->headers->access_control_request_headers, &field)) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;

	return HTTP_SUCCESS;
}

HTTPError http_process_connection(const char* value, HTTPRequest* req) {
	int res = lookup_str_int(value, http_connection_lookup_table, HTTP_CONNECTION_TABLE_COUNT, false);
	if (res == -1) return HTTP_BAD_CONNECTION;
	req->headers->connection = res;

	return HTTP_SUCCESS;
}

HTTPError http_process_cache_control(const char* value, HTTPRequest* req) {
	// todo

	return HTTP_SUCCESS;
}
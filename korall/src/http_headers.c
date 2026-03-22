#include "http_headers.h"
#include "utils.h"
#include "http_internal.h"
#include "lookup_tables.h"
#include "sockets.h"
#include "date.h"

/* General */

/*
	Make sure domain:port or domain is valid format
*/
HTTPError http_domain_port(const char* value, char* domain, char* port, bool *with_port) {
	if (*value == '\0') return HTTP_BAD_DOMAIN_PORT;
	char temp[MAX_DOMAIN_LEN + 1] = { 0 };

	*with_port = false;
	int len = MAX_DOMAIN_LEN;
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

static int http_process_weighted_list(
	const char* value, 
	LookupTable *table,
	char *field_arr, 
	const int field_arr_len
) {

	int i = 0;
	for (char c = *value; ; c = *(++value)) {
		if (!(c == ',' || c == ';' || c == '\0')) {
			field_arr[i] = c;
			i++;
			continue;
		}
		field_arr[i] = '\0';
		int res = lookup_str_int(field_arr, table, false);
		if (res == -1) return -1;

		if (c == ';') {
			// get weight
			++value; // skip ;
			if (!(value[0] == 'q' && value[1] == '=')) return -1; // invalid weight format
			if (!((value[2] == '0' && value[3] == '.') || value[2] == '1')) return -1;
			if (value[2] == '1') {
				if (!(value[3] == '\0' || value[3] == ',')) return -1;
				value += 3;
			}
			else {
				value += 4;

				bool at_least_1_digit = false;
				for (c = *value; c != ',' && c != '\0'; c = *(++value)) {
					if (!is_digit(c)) return -1;
					at_least_1_digit = true;
				}
				if (!at_least_1_digit) return -1;
			}
		}

		if (*value == '\0') return 0;

		do {
			value++;
		} while (*value == ' '); // skip spaces
		i = 0;
		memset(field_arr, 0, field_arr_len);
		field_arr[i] = *value;
		i++;
	}
	return 0;
}

static int http_process_list(
	const char* value,
	LookupTable* table,
	char* field_arr,
	const int field_arr_len
) {
	int i = 0;
	for (char c = *value; ; c = *(++value)) {
		if (!(c == ',' || c == '\0')) {
			field_arr[i] = c;
			i++;
			continue;
		}
		field_arr[i] = '\0';
		int res = lookup_str_int(field_arr, table, false);
		if (res == -1) return -1;

		if (*value == '\0') return 0;

		do {
			value++;
		} while (*value == ' '); // skip spaces
		i = 0;
		memset(field_arr, 0, field_arr_len);
		field_arr[i] = *value;
		i++;
	}
	return 0;
}

HTTPError http_process_content_length(const char* value) {
	int val;
	str_to_int_errno res = str_to_int(&val, value, 10);
	return (res != STR_TO_INT_SUCCESS) ? HTTP_BAD_CONTENT_LENGTH : HTTP_SUCCESS;
}

HTTPError http_process_connection(const char* value) {
	int val = lookup_str_int(value, &http_connection_lookup_table, false);
	return (val == -1) ? HTTP_BAD_CONNECTION : HTTP_SUCCESS;
}

HTTPError http_process_content_type(const char* value) {
	char type[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	int colon = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, ';');
	if (colon == -1) {
		int res = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, '\0');
		if (res == -1) return HTTP_BAD_CONTENT_TYPE;
	}

	int mt = lookup_str_int(type, &http_media_type_lookup_table, false);
	if (mt == -1) return HTTP_BAD_CONTENT_TYPE;

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

		char bnd[MAX_HTTP_BOUNDARY_LEN + 1] = { 0 };
		int boundary_res = fill_string_char(&value, bnd, MAX_HTTP_BOUNDARY_LEN, '\0');
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

		int charset_int = lookup_str_int(charset, &http_charset_lookup_table, true);
		if (charset_int == -1) return HTTP_BAD_CONTENT_TYPE;

		return HTTP_SUCCESS;
	}
}

HTTPError http_process_date(const char* value) {
	char day_name[3 + 1] = { 0 };
	if (fill_string_char(&value, day_name, 3, ',') == -1) return HTTP_BAD_DATE;
	value++; // skip ,
	if (value[0] == '\0') return HTTP_BAD_DATE;
	value++; // skip space

	if (lookup_str_int(day_name, &day_lookup_table, false) == -1) return HTTP_BAD_DATE;


	char day[2 + 1] = { 0 };
	if (fill_string_char(&value, day, 2, ' ') == -1) return HTTP_BAD_DATE;
	value++; // skip space

	int day_num;
	if (str_to_int(&day_num, day, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;


	char month[3 + 1] = { 0 };
	if (fill_string_char(&value, month, 3, ' ') == -1) return HTTP_BAD_DATE;
	value++;

	int month_val;
	if ((month_val = lookup_str_int(month, &month_lookup_table, false)) == -1) return HTTP_BAD_DATE;
	int month_num = month_val + 1;


	char year[4 + 1] = { 0 };
	if (fill_string_char(&value, year, 4, ' ') == -1) return HTTP_BAD_DATE;
	value++;

	int year_num;
	if (str_to_int(&year_num, year, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;

	if (!is_valid_date(day_num, month_num, year_num)) return HTTP_BAD_DATE;



	char hour[2 + 1] = { 0 };
	if (fill_string_char(&value, hour, 2, ':') == -1) return HTTP_BAD_DATE;
	value++;

	int hour_num;
	if (str_to_int(&hour_num, hour, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;

	if (hour_num < 0 || hour_num > 23) return HTTP_BAD_DATE;




	char minute[2 + 1] = { 0 };
	if (fill_string_char(&value, minute, 2, ':') == -1) return HTTP_BAD_DATE;
	value++;

	int minute_num;
	if (str_to_int(&minute_num, minute, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;

	if (minute_num < 0 || minute_num > 59) return HTTP_BAD_DATE;



	char second[2 + 1] = { 0 };
	if (fill_string_char(&value, second, 2, ' ') == -1) return HTTP_BAD_DATE;
	value++;

	int second_num;
	if (str_to_int(&second_num, second, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;

	if (second_num < 0 || second_num > 59) return HTTP_BAD_DATE;


	if (strstr(value, "GMT") == NULL) return HTTP_BAD_DATE;

	return HTTP_SUCCESS;
}

HTTPError http_process_host(const char* value, HTTPRequest* req) {

	char domain[MAX_DOMAIN_LEN + 1] = { 0 };
	char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	bool with_port = false; // needed since function is used elsewhere, here can throw away
	HTTPError res = http_domain_port(value, domain, port, &with_port);
	if (res != HTTP_SUCCESS) return res;

	strncpy(req->host->domain, domain, MAX_DOMAIN_LEN);
	strncpy(req->host->port, port, MAX_PORT_NUM_CHAR_LEN);
	return HTTP_SUCCESS;
}

HTTPError http_process_accept(const char* value) {
	char temp[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	int res = http_process_weighted_list(value, &http_media_type_lookup_table, temp, MAX_MEDIA_TYPE_LEN + 1);
	if (res == -1) return HTTP_BAD_ACCEPT;
	return HTTP_SUCCESS;
}

HTTPError http_process_accept_encoding(const char* value) {
	char temp[MAX_ENCODING_CHAR_LEN + 1] = { 0 };
	int res = http_process_weighted_list(value, &http_encoding_lookup_table, temp, MAX_ENCODING_CHAR_LEN + 1);
	if (res == -1) return HTTP_BAD_ACCEPT_ENC;
	return HTTP_SUCCESS;
}

HTTPError http_process_access_control_request_method(const char* value) {
	int res = lookup_str_int(value, &http_method_lookup_table, true);
	if (res == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_METHOD;
	
	return HTTP_SUCCESS;
}

HTTPError http_process_access_control_request_headers(const char* value) {
	char val[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	HTTPRequestHeaderField field;
	while (fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, ',') != -1 || fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, '\0') != -1) {
		if ((field = lookup_str_int(val, &http_req_header_field_lookup_table, true)) == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
		if (value[0] == '\0') return HTTP_SUCCESS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, MAX_HTTP_HEADER_FIELD_LEN);
	};

	return HTTP_SUCCESS;
}

HTTPError http_process_cache_control_req(const char* value) {

	char val[MAX_HTTP_REQ_CC_LEN + 1] = { 0 };
	size_t len = MAX_HTTP_REQ_CC_LEN;
	int name;
	while (
		fill_string_char(&value, val, len, '=') != -1
		|| fill_string_char(&value, val, len, ',') != -1
		|| fill_string_char(&value, val, len, '\0') != -1
	) {
		if ((name = lookup_str_int(val, &http_req_cache_control_lookup_table, true)) == -1) return HTTP_BAD_CACHE_CONTROL;
		if (value[0] == '=') {
			if (!HTTP_REQ_CC_HAS_VAL(name)) return HTTP_BAD_CACHE_CONTROL;
			value++; // skip =
			char sec[24 + 1] = { 0 }; // todo: change 24
			size_t sec_len = 24;

			if (!(fill_string_char(&value, sec, sec_len, ',') != -1 || fill_string_char(&value, sec, sec_len, '\0') != -1)) return HTTP_BAD_CACHE_CONTROL;
			
			int sec_num;
			str_to_int_errno res = str_to_int(&sec_num, sec, 10);
			if (res != STR_TO_INT_SUCCESS) return HTTP_BAD_CACHE_CONTROL;
		}
				
		if (value[0] == '\0') return HTTP_SUCCESS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, len + 1);
	};

	return HTTP_SUCCESS;
}

HTTPError http_process_user_agent(const char* value) {
	return HTTP_SUCCESS;
}

HTTPError http_process_expect(const char* value) {
	if (strncmp(value, "100-continue", 13) != 0) return HTTP_BAD_EXPECT;
	return HTTP_SUCCESS;
}

HTTPError http_process_te(const char* value) {
	char temp[MAX_TE_LEN + 1] = { 0 };
	int res = http_process_weighted_list(value, &http_te_lookup_table, temp, MAX_TE_LEN + 1);
	if (res == -1) return HTTP_BAD_TE;
	return HTTP_SUCCESS;
}

HTTPError http_process_transfer_encoding(const char* value) {
	char temp[MAX_TRANSFER_ENCODING_LEN + 1] = { 0 };
	int res = http_process_list(value, &http_transfer_encoding_lookup_table, temp, MAX_TRANSFER_ENCODING_LEN + 1);
	if (res == -1) return HTTP_BAD_TRANSFER_ENCODING;
	return HTTP_SUCCESS;
}

HTTPError http_process_server(const char* value) {
	// todo: 
	return HTTP_SUCCESS;
}

HTTPError http_process_max_forwards(const char* value) {
	int val;
	str_to_int_errno res = str_to_int(&val, value, 10);
	if (res != STR_TO_INT_SUCCESS || val < 0) return HTTP_BAD_MAX_FORWARDS;
	return HTTP_SUCCESS;
}

HTTPError http_process_tk(const char* value) {
	if (value[0] == '\0' || value[1] != '\0') return HTTP_BAD_TK;
	char c = value[0];
	if (
		c == '!' ||
		c == '?' ||
		c == 'G' ||
		c == 'N' ||
		c == 'T' ||
		c == 'C' ||
		c == 'P' ||
		c == 'D' ||
		c == 'U'
		) return HTTP_SUCCESS;
	return HTTP_BAD_TK;
}

/* Response */

HTTPError http_process_cache_control_res(const char* value) {
	char val[MAX_HTTP_RES_CC_LEN + 1] = { 0 };
	HTTPResponseCacheControl name;
	size_t len = MAX_HTTP_RES_CC_LEN;
	while (
		fill_string_char(&value, val, len, '=') != -1
		|| fill_string_char(&value, val, len, ',') != -1
		|| fill_string_char(&value, val, len, '\0') != -1
		) {
		if ((name = lookup_str_int(val, &http_res_cache_control_lookup_table, true)) == -1) return HTTP_BAD_CACHE_CONTROL;
		if (value[0] == '=') {
			if (!HTTP_RES_CC_HAS_VAL(name)) return HTTP_BAD_CACHE_CONTROL;
			value++; // skip =
			char sec[24 + 1] = { 0 }; // todo: change 24
			size_t sec_len = 24;

			if (!(fill_string_char(&value, sec, sec_len, ',') != -1 || fill_string_char(&value, sec, sec_len, '\0') != -1)) return HTTP_BAD_CACHE_CONTROL;

			int sec_num;
			str_to_int_errno res = str_to_int(&sec_num, sec, 10);
			if (res != STR_TO_INT_SUCCESS) return HTTP_BAD_CACHE_CONTROL;
		}

		if (value[0] == '\0') return HTTP_SUCCESS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, len + 1);
	};

	return HTTP_SUCCESS;
}

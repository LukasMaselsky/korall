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
	HTTPRequest* req, 
	LookupTable *table,
	char *field_arr, 
	const int field_arr_len, 
	Array* res_arr
) {
	int i = 0;
	for (char c = *value; ; c = *(++value)) {
		if (c == ',' || c == ';' || c == '\0') {
			field_arr[i] = '\0';
			int res = lookup_str_int(field_arr, table, false);
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

static HTTPError http_process_content_length(const char* value, int *val) {
	str_to_int_errno res = str_to_int(val, value, 10);
	return (res != STR_TO_INT_SUCCESS) ? HTTP_BAD_CONTENT_LENGTH : HTTP_SUCCESS;
}

static HTTPError http_process_connection(const char* value, int *val) {
	*val = lookup_str_int(value, &http_connection_lookup_table, false);
	return (*val == -1) ? HTTP_BAD_CONNECTION : HTTP_SUCCESS;
}

static HTTPError http_process_content_type(const char* value, HTTPMediaType *media_type, char *boundary, HTTPCharset *chst) {
	char type[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	int colon = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, ';');
	if (colon == -1) {
		int res = fill_string_char(&value, type, MAX_MEDIA_TYPE_LEN, '\0');
		if (res == -1) return HTTP_BAD_CONTENT_TYPE;
	}

	int mt = lookup_str_int(type, &http_media_type_lookup_table, false);
	if (mt == -1) return HTTP_BAD_CONTENT_TYPE;

	*media_type = mt;
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

		int boundary_res = fill_string_char(&value, boundary, MAX_HTTP_BOUNDARY_LEN, '\0');
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

		*chst = charset_int;
		return HTTP_SUCCESS;
	}
}

static HTTPError http_process_date(const char* value, 
	Day *day_name_val, 
	Month *month_val,
	uint16_t *year_val,
	byte *day_val,
	byte *hour_val,
	byte *minute_val,
	byte *second_val
) {
	char day_name[3 + 1] = { 0 };
	if (fill_string_char(&value, day_name, 3, ',') == -1) return HTTP_BAD_DATE;
	value++; // skip ,
	if (value[0] == '\0') return HTTP_BAD_DATE;
	value++; // skip space

	if ((*day_name_val = lookup_str_int(day_name, &day_lookup_table, false)) == -1) return HTTP_BAD_DATE;


	char day[2 + 1] = { 0 };
	if (fill_string_char(&value, day, 2, ' ') == -1) return HTTP_BAD_DATE;
	value++; // skip space

	int day_num;
	if (str_to_int(&day_num, day, 10) != STR_TO_INT_SUCCESS) return HTTP_BAD_DATE;


	char month[3 + 1] = { 0 };
	if (fill_string_char(&value, month, 3, ' ') == -1) return HTTP_BAD_DATE;
	value++;

	if ((*month_val = lookup_str_int(month, &month_lookup_table, false)) == -1) return HTTP_BAD_DATE;
	int month_num = *month_val + 1;


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


	*year_val = (uint16_t)day_num;
	*day_val = (byte)year_num;
	*hour_val = (byte)hour_num;
	*minute_val = (byte)minute_num;
	*second_val = (byte)second_num;

	return HTTP_SUCCESS;
}

/* Request */

HTTPError http_process_host_req(const char* value, HTTPRequest* req) {

	char domain[MAX_DOMAIN_LEN + 1] = { 0 };
	char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	bool with_port = false; // needed since function is used elsewhere, here can throw away
	HTTPError res = http_domain_port(value, domain, port, &with_port);
	if (res != HTTP_SUCCESS) return res;

	strncpy(req->headers->host->domain, domain, MAX_DOMAIN_LEN);
	strncpy(req->headers->host->port, port, MAX_PORT_NUM_CHAR_LEN);
	return HTTP_SUCCESS;
}

HTTPError http_process_accept_req(const char* value, HTTPRequest* req) {
	char temp[MAX_MEDIA_TYPE_LEN + 1] = { 0 };
	Array* res_arr = req->headers->accept;
	int res = http_process_weighted_list(value, req, &http_media_type_lookup_table, temp, MAX_MEDIA_TYPE_LEN + 1, res_arr);
	if (res == -1) return HTTP_BAD_ACCEPT;
	return HTTP_SUCCESS;
}

HTTPError http_process_accept_encoding_req(const char* value, HTTPRequest* req) {
	char temp[MAX_ENCODING_CHAR_LEN + 1] = { 0 };
	Array* res_arr = req->headers->accept_encoding;
	int res = http_process_weighted_list(value, req, &http_encoding_lookup_table, temp, MAX_ENCODING_CHAR_LEN + 1, res_arr);
	if (res == -1) return HTTP_BAD_ACCEPT_ENC;
	return HTTP_SUCCESS;
}

HTTPError http_process_content_length_req(const char* value, HTTPRequest* req) {
	return http_process_content_length(value, &(req->headers->content_length));
}

HTTPError http_process_content_type_req(const char* value, HTTPRequest* req) {
	return http_process_content_type(value, 
		&(req->headers->content_type->media_type), 
		req->headers->content_type->boundary, 
		&(req->headers->content_type->charset)
	);
}

HTTPError http_process_access_control_request_method_req(const char* value, HTTPRequest* req) {
	int res = lookup_str_int(value, &http_method_lookup_table, true);
	if (res == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_METHOD;
	req->headers->access_control_request_method = res;

	return HTTP_SUCCESS;
}

HTTPError http_process_access_control_request_headers_req(const char* value, HTTPRequest* req) {
	char val[MAX_HTTP_HEADER_FIELD_LEN + 1] = { 0 };
	HTTPRequestHeaderField field;
	while (fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, ',') != -1 || fill_string_char(&value, val, MAX_HTTP_HEADER_FIELD_LEN, '\0') != -1) {
		if ((field = lookup_str_int(val, &http_req_header_field_lookup_table, true)) == -1) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
		if (!array_add(req->headers->access_control_request_headers, &field)) return HTTP_BAD_ACCESS_CONTROL_REQUEST_HEADERS;
		if (value[0] == '\0') return HTTP_SUCCESS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, MAX_HTTP_HEADER_FIELD_LEN + 1);
	};

	return HTTP_SUCCESS;
}

HTTPError http_process_connection_req(const char* value, HTTPRequest* req) {
	return http_process_connection(value, &(req->headers->connection));
}

HTTPError http_process_cache_control_req(const char* value, HTTPRequest* req) {

	char val[MAX_HTTP_REQ_CC_LEN + 1] = { 0 };
	HTTPRequestCacheControlPair field = { .name = HTTP_REQ_CC_UNUSED, .seconds = -1 };
	size_t len = MAX_HTTP_REQ_CC_LEN;
	while (
		fill_string_char(&value, val, len, '=') != -1
		|| fill_string_char(&value, val, len, ',') != -1
		|| fill_string_char(&value, val, len, '\0') != -1
	) {
		if ((field.name = lookup_str_int(val, &http_req_cache_control_lookup_table, true)) == -1) return HTTP_BAD_CACHE_CONTROL;
		if (value[0] == '=') {
			if (!HTTP_REQ_CC_HAS_VAL(field.name)) return HTTP_BAD_CACHE_CONTROL;
			value++; // skip =
			char sec[24 + 1] = { 0 }; // todo: change 24
			size_t sec_len = 24;

			if (!(fill_string_char(&value, sec, sec_len, ',') != -1 || fill_string_char(&value, sec, sec_len, '\0') != -1)) return HTTP_BAD_CACHE_CONTROL;
			
			int sec_num;
			str_to_int_errno res = str_to_int(&sec_num, sec, 10);
			if (res != STR_TO_INT_SUCCESS) return HTTP_BAD_CACHE_CONTROL;
			field.seconds = sec_num;
		}
		
		if (!array_add(req->headers->cache_control, &field)) return HTTP_BAD_CACHE_CONTROL;
		
		if (value[0] == '\0') return HTTP_SUCCESS;
		value++; // skip ,
		while (value[0] == ' ')
			value++;
		memset(val, 0, len + 1);

		// reset field temp
		field.name = HTTP_REQ_CC_UNUSED;
		field.seconds = -1;
	};

	return HTTP_SUCCESS;
}

HTTPError http_process_user_agent_req(const char* value, HTTPRequest* req) {
	if (fill_string_char(&value, req->headers->user_agent, MAX_HTTP_USER_AGENT, '\0') == -1) return HTTP_BAD_USER_AGENT;
	return HTTP_SUCCESS;
}

HTTPError http_process_date_req(const char* value, HTTPRequest* req) {
	HTTPDate* date = req->headers->date;
	return http_process_date(value, &(date->day_name), &(date->month), &(date->year), &(date->day), &(date->hour), &(date->minute), &(date->second));
}

HTTPError http_process_expect_req(const char* value, HTTPRequest* req) {
	if (strncmp(value, "100-continue", 13) != 0) return HTTP_BAD_EXPECT;
	req->headers->expect = HTTP_EXP_100_CONTINUE;
	return HTTP_SUCCESS;
}

/* Response */

HTTPError http_process_content_length_res(const char* value) {
	int val;
	return http_process_content_length(value, &val);
}

HTTPError http_process_content_type_res(const char* value) {
	HTTPMediaType mt;
	HTTPCharset ct;
	char bd[MAX_HTTP_BOUNDARY_LEN + 1] = { 0 };
	return http_process_content_type(value, &mt, bd, &ct);
}

HTTPError http_process_connection_res(const char* value) {
	int val;
	return http_process_connection(value, &val);
}

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

HTTPError http_process_server_res(const char* value) {
	size_t len = strlen(value);
	if (len > MAX_HTTP_HEADER_VALUE_LEN) return HTTP_BAD_SERVER;
	return HTTP_SUCCESS;
}

HTTPError http_process_date_res(const char* value) {
	Day day_n;
	Month month;
	uint16_t year;
	byte hour, minute, second, day;
	return http_process_date(value, &day_n, &month, &year, &day, &hour, &minute, &second);
}

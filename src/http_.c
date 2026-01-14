#include "http_.h"
#include "utils.h"
#include "lookup_tables.h"

HTTPMethod process_http_method(char *str, LookupEntry *table, int table_len) {
	
	char method[MAX_HTTP_METHOD_STR_LEN];
	int len = MAX_HTTP_METHOD_STR_LEN;
	int i = 0;
	for (char c = *str; c != '\0' && c != ' ' && i < len - 1; c = *(++str)) {
		method[i] = c;
		i++;
	}
	method[i] = '\0';
	return lookup(method, table, table_len);
}
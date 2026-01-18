#include "http_.h"
#include "lookup_tables.h"

Flags default_flags = {
	.servertype = ST_TCP,
};

const LookupEntry flag_lookup_table[FLAG_LOOKUP_TABLE_COUNT] = {
	{"tcp", F_TCP},
	{"http", F_HTTP},
};

const LookupEntry http_method_lookup_table[HTTP_METHOD_LOOKUP_TABLE_COUNT] = {
	{ "CONNECT", HTTP_CONNECT },
	{ "DELETE", HTTP_DELETE },
	{ "GET", HTTP_GET },
	{ "HEAD", HTTP_HEAD },
	{ "OPTIONS", HTTP_OPTIONS },
	{ "PATCH", HTTP_PATCH },
	{ "POST", HTTP_POST },
	{ "PUT", HTTP_PUT },
	{ "TRACE" , HTTP_TRACE }
};

const LookupEntry http_header_field_lookup_table[HTTP_HEADER_FIELD_TABLE_COUNT] = {
	{"Host", HTTP_H_HOST},
};
#include "http_.h"
#include "lookup_tables.h"

Flags default_flags = {
	.server_type = ST_HTTP,
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
	{"A-IM", HTTP_H_A_IM},
	{"Accept", HTTP_H_ACCEPT},
	{"Accept-Charset", HTTP_H_ACCEPT_CHARSET},
	{"Accept-Datetime", HTTP_H_ACCEPT_DATETIME},
	{"Accept-Encoding", HTTP_H_ACCEPT_ENCODING},
	{"Accept-Language", HTTP_H_ACCEPT_LANGUAGE},
	{"Access-Control-Request-Method", HTTP_H_ACCESS_CONTROL_REQUEST_METHOD},
	{"Access-Control-Request-Headers", HTTP_H_ACCESS_CONTROL_REQUEST_HEADERS},
	{"Authorization", HTTP_H_AUTHORIZATION},
	{"Cache-Control", HTTP_H_CACHECONTROL},
	{"Connection", HTTP_H_CONNECTION},
	{"Encoding", HTTP_H_CONTENT_ENCODING},
	{"Content-Length", HTTP_H_CONTENT_LENGTH},
	{"Content-MD5", HTTP_H_CONTENT_MD5},
	{"Content-Type", HTTP_H_CONTENT_TYPE},
	//{"Cookie", HTTP_H_COOKIE},
	{"Date", HTTP_H_DATE},
	{"Expect", HTTP_H_EXPECT},
	{"Forwarded", HTTP_H_FORWARDED},
	{"From", HTTP_H_FROM},
	{"Host", HTTP_H_HOST},
	{"HTTP2-Setting", HTTP_H_HTTP2_SETTINGS},
	{"If-Match", HTTP_H_IF_MATCH},
	{"If-Modified-Since", HTTP_H_IF_MODIFIED_SINCE},
	{"If-None-Match", HTTP_H_IF_NONE_MATCH},
	{"If-Range", HTTP_H_IF_RANGE},
	{"If-Unmodified-Since", HTTP_H_IF_UNMODIFIED_SINCE},
	{"Max-Forwards", HTTP_H_MAX_FORWARDS},
	{"Origin", HTTP_H_ORIGIN},
	{"Pragma", HTTP_H_PRAGMA},
	{"Prefer", HTTP_H_PREFER},
	{"Proxy-Authorization", HTTP_H_PROXY_AUTHORIZATION},
	{"Range", HTTP_H_RANGE},
	{"Referer", HTTP_H_REFERER},
	{"TE", HTTP_H_TE},
	{"Trailer", HTTP_H_TRAILER},
	{"Transfer-Encoding", HTTP_H_TRANSFER_ENCODING},
	{"User-Agent", HTTP_H_USER_AGENT},
	{"Upgrade", HTTP_H_UPGRADE},
	{"Via", HTTP_H_VIA},
	{"Warning", HTTP_H_WARNING},
};
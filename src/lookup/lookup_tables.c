#include "lookup_tables.h"

int lookup_str_int(const char* key, const LookupTable* table, const bool case_insensitive) {
	if (key[0] == '\0') return -1;
	for (const LookupEntry* entry = table->entries; entry != table->entries + table->size; entry++) {
		if (case_insensitive) {
			if (strcmp_ci(entry->string, key) == 0)
				return entry->integer;
		}
		else {
			if (*(entry->string) != *key) continue;

			if (strcmp(entry->string, key) == 0)
				return entry->integer;
		}
	}

	return -1;
}

const char* lookup_int_str(const int key, const LookupTable* table) {
	for (const LookupEntry* entry = table->entries; entry != table->entries + table->size; entry++) {
		if (entry->integer == key)
			return entry->string;
	}

	return NULL;
}

const LookupEntry http_method_lookup_table_entries[HTTP_METHOD_TABLE_COUNT] = {
	{ HTTP_CONNECT, "CONNECT" },
	{ HTTP_DELETE, "DELETE" },
	{ HTTP_GET, "GET" },
	{ HTTP_HEAD, "HEAD" },
	{ HTTP_OPTIONS, "OPTIONS" },
	{ HTTP_PATCH, "PATCH" },
	{ HTTP_POST, "POST" },
	{ HTTP_PUT, "PUT" },
	{ HTTP_TRACE, "TRACE" }
};

const LookupEntry http_req_header_field_lookup_table_entries[HTTP_REQ_HEADER_FIELD_TABLE_COUNT] = {
	{ HTTP_RQH_A_IM, "A-IM" },
	{ HTTP_RQH_ACCEPT, "Accept" },
	{ HTTP_RQH_ACCEPT_CHARSET, "Accept-Charset" },
	{ HTTP_RQH_ACCEPT_DATETIME, "Accept-Datetime" },
	{ HTTP_RQH_ACCEPT_ENCODING, "Accept-Encoding" },
	{ HTTP_RQH_ACCEPT_LANGUAGE, "Accept-Language" },
	{ HTTP_RQH_ACCESS_CONTROL_REQUEST_METHOD, "Access-Control-Request-Method" },
	{ HTTP_RQH_ACCESS_CONTROL_REQUEST_HEADERS, "Access-Control-Request-Headers" },
	{ HTTP_RQH_AUTHORIZATION, "Authorization" },
	{ HTTP_RQH_CACHE_CONTROL, "Cache-Control" },
	{ HTTP_RQH_CONNECTION, "Connection" },
	{ HTTP_RQH_CONTENT_ENCODING, "Content-Encoding" },
	{ HTTP_RQH_CONTENT_LENGTH, "Content-Length" },
	{ HTTP_RQH_CONTENT_MD5, "Content-MD5" },
	{ HTTP_RQH_CONTENT_TYPE, "Content-Type" },
	// { HTTP_RQH_COOKIE, "Cookie" },
	{ HTTP_RQH_DATE, "Date" },
	{ HTTP_RQH_EXPECT, "Expect" },
	{ HTTP_RQH_FORWARDED, "Forwarded" },
	{ HTTP_RQH_FROM, "From" },
	{ HTTP_RQH_HOST, "Host" },
	{ HTTP_RQH_HTTP2_SETTINGS, "HTTP2-Setting" },
	{ HTTP_RQH_IF_MATCH, "If-Match" },
	{ HTTP_RQH_IF_MODIFIED_SINCE, "If-Modified-Since" },
	{ HTTP_RQH_IF_NONE_MATCH, "If-None-Match" },
	{ HTTP_RQH_IF_RANGE, "If-Range" },
	{ HTTP_RQH_IF_UNMODIFIED_SINCE, "If-Unmodified-Since" },
	{ HTTP_RQH_MAX_FORWARDS, "Max-Forwards" },
	{ HTTP_RQH_ORIGIN, "Origin" },
	{ HTTP_RQH_PRAGMA, "Pragma" },
	{ HTTP_RQH_PREFER, "Prefer" },
	{ HTTP_RQH_PROXY_AUTHORIZATION, "Proxy-Authorization" },
	{ HTTP_RQH_RANGE, "Range" },
	{ HTTP_RQH_REFERER, "Referer" },
	{ HTTP_RQH_TE, "TE" },
	{ HTTP_RQH_TRAILER, "Trailer" },
	{ HTTP_RQH_TRANSFER_ENCODING, "Transfer-Encoding" },
	{ HTTP_RQH_USER_AGENT, "User-Agent" },
	{ HTTP_RQH_UPGRADE, "Upgrade" },
	{ HTTP_RQH_VIA, "Via" },
	{ HTTP_RQH_WARNING, "Warning" },
	{ HTTP_RQH_WS_KEY, "Sec-WebSocket-Key" },
	{ HTTP_RQH_WS_VERSION, "Sec-WebSocket-Version" },
};

const LookupEntry http_status_code_lookup_table_entries[HTTP_STATUS_CODE_TABLE_COUNT] = {
	{ HTTP_SC_100, "Continue"},
	{ HTTP_SC_101, "Switching Protocols"},
	{ HTTP_SC_102, "Processing"},
	{ HTTP_SC_103, "Early Hints"},
	{ HTTP_SC_200, "OK"},
	{ HTTP_SC_201, "Created"},
	{ HTTP_SC_202, "Accepted"},
	{ HTTP_SC_203, "Non-Authoritative Information"},
	{ HTTP_SC_204, "No Content"},
	{ HTTP_SC_205, "Reset Content"},
	{ HTTP_SC_206, "Partial Content"},
	{ HTTP_SC_207, "Multi-Status"},
	{ HTTP_SC_208, "Already Reported"},
	{ HTTP_SC_226, "IM Used"},
	{ HTTP_SC_300, "Multiple Choices"},
	{ HTTP_SC_301, "Moved Permanently"},
	{ HTTP_SC_302, "Found"},
	{ HTTP_SC_303, "See Other"},
	{ HTTP_SC_304, "Not Modified"},
	{ HTTP_SC_305, "Use Proxy"},
	{ HTTP_SC_307, "Temporary Redirect"},
	{ HTTP_SC_308, "Permanent Redirect"},
	{ HTTP_SC_400, "Bad Request"},
	{ HTTP_SC_401, "Unauthorized"},
	{ HTTP_SC_402, "Payment Required"},
	{ HTTP_SC_403, "Forbidden"},
	{ HTTP_SC_404, "Not Found"},
	{ HTTP_SC_405, "Method Not Allowed"},
	{ HTTP_SC_406, "Not Acceptable"},
	{ HTTP_SC_407, "Proxy Authentication Required"},
	{ HTTP_SC_408, "Request Timeout"},
	{ HTTP_SC_409, "Conflict"},
	{ HTTP_SC_410, "Gone"},
	{ HTTP_SC_411, "Length Required"},
	{ HTTP_SC_412, "Precondition Failed"},
	{ HTTP_SC_413, "Payload Too Large"},
	{ HTTP_SC_414, "URI Too Long"},
	{ HTTP_SC_415, "Unsupported Media Type"},
	{ HTTP_SC_416, "Requested Range Not Satisfiable"},
	{ HTTP_SC_417, "Expectation Failed"},
	{ HTTP_SC_418, "I'm a teapot"},
	{ HTTP_SC_421, "Misdirected Request"},
	{ HTTP_SC_422, "Unprocessable Entity"},
	{ HTTP_SC_423, "Locked"},
	{ HTTP_SC_424, "Failed Dependency"},
	{ HTTP_SC_425, "Too Early"},
	{ HTTP_SC_426, "Upgrade Required"},
	{ HTTP_SC_428, "Precondition Required"},
	{ HTTP_SC_429, "Too Many Requests"},
	{ HTTP_SC_431, "Request Header Fields Too Large"},
	{ HTTP_SC_444, "Connection Closed Without Response"},
	{ HTTP_SC_451, "Unavailable For Legal Reasons"},
	{ HTTP_SC_499, "Client Closed Request"},
	{ HTTP_SC_500, "Internal Server Error"},
	{ HTTP_SC_501, "Not Implemented"},
	{ HTTP_SC_502, "Bad Gateway"},
	{ HTTP_SC_503, "Service Unavailable"},
	{ HTTP_SC_504, "Gateway Timeout"},
	{ HTTP_SC_505, "HTTP Version Not Supported"},
	{ HTTP_SC_506, "Variant Also Negotiates"},
	{ HTTP_SC_507, "Insufficient Storage"},
	{ HTTP_SC_508, "Loop Detected"},
	{ HTTP_SC_510, "Not Extended"},
	{ HTTP_SC_511, "Network Authentication Required"},
	{ HTTP_SC_599, "Network Connect Timeout Error"}
};

const LookupEntry http_res_header_field_lookup_table_entries[HTTP_RES_HEADER_FIELD_TABLE_COUNT] = {
	{ HTTP_RSH_ACCEPT_CH, "Accept-CH" },
	{ HTTP_RSH_ACCESS_CONTROL_ALLOW_ORIGIN, "Access-Control-Allow-Origin" },
	{ HTTP_RSH_ACCESS_CONTROL_ALLOW_CREDENTIALS, "Access-Control-Allow-Credentials" },
	{ HTTP_RSH_ACCESS_CONTROL_EXPOSE_HEADERS, "Access-Control-Expose-Headers" },
	{ HTTP_RSH_ACCESS_CONTROL_MAX_AGE, "Access-Control-Max-Age" },
	{ HTTP_RSH_ACCESS_CONTROL_ALLOW_METHODS, "Access-Control-Allow-Methods" },
	{ HTTP_RSH_ACCESS_CONTROL_ALLOW_HEADERS, "Access-Control-Allow-Headers" },
	{ HTTP_RSH_ACCEPT_PATCH, "Accept-Patch" },
	{ HTTP_RSH_ACCEPT_RANGES, "Accept-Ranges" },
	{ HTTP_RSH_AGE, "Age" },
	{ HTTP_RSH_ALLOW, "Allow" },
	{ HTTP_RSH_ALT_SVC, "Alt-Svc" },
	{ HTTP_RSH_CACHE_CONTROL, "Cache-Control" },
	{ HTTP_RSH_CONNECTION, "Connection" },
	{ HTTP_RSH_CONTENT_DISPOSITION, "Content-Disposition" },
	{ HTTP_RSH_CONTENT_ENCODING, "Content-Encoding" },
	{ HTTP_RSH_CONTENT_LANGUAGE, "Content-Language" },
	{ HTTP_RSH_CONTENT_LENGTH, "Content-Length" },
	{ HTTP_RSH_CONTENT_LOCATION, "Content-Location" },
	{ HTTP_RSH_CONTENT_MD5, "Content-MD5" },
	{ HTTP_RSH_CONTENT_RANGE, "Content-Range" },
	{ HTTP_RSH_CONTENT_TYPE, "Content-Type" },
	{ HTTP_RSH_DATE, "Date" },
	{ HTTP_RSH_DELTA_BASE, "Delta-Base" },
	{ HTTP_RSH_ETAG, "ETag" },
	{ HTTP_RSH_EXPIRES, "Expires" },
	{ HTTP_RSH_IM, "IM" },
	{ HTTP_RSH_LAST_MODIFIED, "Last-Modified" },
	{ HTTP_RSH_LINK, "Link" },
	{ HTTP_RSH_LOCATION, "Location" },
	{ HTTP_RSH_P3P, "P3P" },
	{ HTTP_RSH_PRAGMA, "Pragma" },
	{ HTTP_RSH_PREFERENCE_APPLIED, "Preference-Applied" },
	{ HTTP_RSH_PROXY_AUTHENTICATE, "Proxy-Authenticate" },
	{ HTTP_RSH_PUBLIC_KEY_PINS, "Public-Key-Pins" },
	{ HTTP_RSH_RETRY_AFTER, "Retry-After" },
	{ HTTP_RSH_SERVER, "Server" },
	{ HTTP_RSH_SET_COOKIE, "Set-Cookie" },
	{ HTTP_RSH_STRICT_TRANSPORT_SECURITY, "Strict-Transport-Security" },
	{ HTTP_RSH_TRAILER, "Trailer" },
	{ HTTP_RSH_TRANSFER_ENCODING, "Transfer-Encoding" },
	{ HTTP_RSH_TK, "Tk" },
	{ HTTP_RSH_UPGRADE, "Upgrade" },
	{ HTTP_RSH_VARY, "Vary" },
	{ HTTP_RSH_VIA, "Via" },
	{ HTTP_RSH_WARNING, "Warning" },
	{ HTTP_RSH_WWW_AUTHENTICATE, "WWW-Authenticate" },
	{ HTTP_RSH_X_FRAME_OPTIONS, "X-Frame-Options" },
};

const LookupEntry http_media_type_lookup_table_entries[HTTP_MEDIA_TYPE_TABLE_COUNT] = {
	{ HTTP_MT_ANY, "*/*" },
	{ HTTP_MT_APP, "application/*" },
	{ HTTP_MT_APP_JSON, "application/json" },
	{ HTTP_MT_APP_LD_JSON, "application/ld+json" },
	{ HTTP_MT_APP_MSWORD, "application/msword" },
	{ HTTP_MT_APP_PDF, "application/pdf" },
	{ HTTP_MT_APP_SQL, "application/sql" },
	{ HTTP_MT_APP_VND_API_JSON, "application/vnd.api+json" },
	{ HTTP_MT_APP_VND_MS_PORT_EXEC, "application/vnd.microsoft.portable-executable" },
	{ HTTP_MT_APP_VND_MS_XLS, "application/vnd.ms-excel" },
	{ HTTP_MT_APP_VND_MS_PPT, "application/vnd.ms-powerpoint" },
	{ HTTP_MT_APP_VND_ODT, "application/vnd.oasis.opendocument.text" },
	{ HTTP_MT_APP_VND_PPTX, "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
	{ HTTP_MT_APP_VND_XLSX, "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
	{ HTTP_MT_APP_VND_DOCX, "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
	{ HTTP_MT_APP_X_WWW_FORM_URLENCODED, "application/x-www-form-urlencoded" },
	{ HTTP_MT_APP_XML, "application/xml" },
	{ HTTP_MT_APP_XHTML_XML, "application/xhtml+xml"},
	{ HTTP_MT_APP_ZIP, "application/zip" },
	{ HTTP_MT_APP_ZSTD, "application/zstd" },
	{ HTTP_MT_AUD, "audio/*" },
	{ HTTP_MT_AUD_MPEG, "audio/mpeg" },
	{ HTTP_MT_AUD_OGG, "audio/ogg" },
	{ HTTP_MT_IMG, "image/*" },
	{ HTTP_MT_IMG_AVIF, "image/avif" },
	{ HTTP_MT_IMG_WEBP, "image/webp" },
	{ HTTP_MT_IMG_JPEG, "image/jpeg" },
	{ HTTP_MT_IMG_PNG, "image/png" },
	{ HTTP_MT_IMG_SVG_XML, "image/svg+xml" },
	{ HTTP_MT_IMG_TIFF, "image/tiff" },
	{ HTTP_MT_MOD, "model/*" },
	{ HTTP_MT_MOD_OBJ, "model/obj" },
	{ HTTP_MT_MTP, "multipart/*" },
	{ HTTP_MT_MTP_FORM_DATA, "multipart/form-data" },
	{ HTTP_MT_TXT, "text/*" },
	{ HTTP_MT_TXT_PLAIN, "text/plain" },
	{ HTTP_MT_TXT_CSS, "text/css" },
	{ HTTP_MT_TXT_CSV, "text/csv" },
	{ HTTP_MT_TXT_HTML, "text/html" },
	{ HTTP_MT_TXT_JS, "text/javascript" },
	{ HTTP_MT_TXT_XML, "text/xml" },
};	  

const LookupEntry http_accept_encoding_lookup_table_entries[HTTP_ACCEPT_ENCODING_TABLE_COUNT] = {
	{ HTTP_ACCENC_ANY, "*" },
	{ HTTP_ACCENC_GZIP, "gzip" },
	{ HTTP_ACCENC_COMPRESS, "compress" },
	{ HTTP_ACCENC_DEFLATE, "deflate" },
	{ HTTP_ACCENC_BR, "br" },
	{ HTTP_ACCENC_ZSTD, "zstd" },
	{ HTTP_ACCENC_DCB, "dcb" },
	{ HTTP_ACCENC_DCZ, "dcz" },
	{ HTTP_ACCENC_IDENTITY, "identity" },
};

const LookupEntry http_te_lookup_table_entries[HTTP_TE_TABLE_COUNT] = {
	{ HTTP_TE_ANY, "*"},
	{ HTTP_TE_GZIP, "gzip"},
	{ HTTP_TE_COMPRESS, "compress"},
	{ HTTP_TE_DEFLATE, "deflate"},
	{ HTTP_TE_TRAILERS, "trailers"},
};

const LookupEntry http_transfer_encoding_lookup_table_entries[HTTP_TRANSFER_ENCODING_TABLE_COUNT] = {
	{ HTTP_TRENC_ANY, "*"},
	{ HTTP_TRENC_GZIP, "gzip"},
	{ HTTP_TRENC_COMPRESS, "compress"},
	{ HTTP_TRENC_DEFLATE, "deflate"},
	{ HTTP_TRENC_CHUNKED, "chunked"},
};

const LookupEntry http_charset_lookup_table_entries[HTTP_CHARSET_TABLE_COUNT] = {
	{ HTTP_CHS_BIG5, "big5" },
	{ HTTP_CHS_EUC_KR, "euc-kr" },
	{ HTTP_CHS_ISO_8859_1, "iso-8859-1" },
	{ HTTP_CHS_ISO_8859_2, "iso-8859-2" },
	{ HTTP_CHS_ISO_8859_3, "iso-8859-3" },
	{ HTTP_CHS_ISO_8859_4, "iso-8859-4" },
	{ HTTP_CHS_ISO_8859_5, "iso-8859-5" },
	{ HTTP_CHS_ISO_8859_6, "iso-8859-6" },
	{ HTTP_CHS_ISO_8859_7, "iso-8859-7" },
	{ HTTP_CHS_ISO_8859_8, "iso-8859-8" },
	{ HTTP_CHS_KOI8_R, "koi8-r" },
	{ HTTP_CHS_SHIFT_JIS, "shift-jis" },
	{ HTTP_CHS_X_EUC, "x-euc" },
	{ HTTP_CHS_UTF_8, "utf-8" },
	{ HTTP_CHS_WINDOWS_1250, "windows-1250" },
	{ HTTP_CHS_WINDOWS_1251, "windows-1251" },
	{ HTTP_CHS_WINDOWS_1252, "windows-1252" },
	{ HTTP_CHS_WINDOWS_1253, "windows-1253" },
	{ HTTP_CHS_WINDOWS_1254, "windows-1254" },
	{ HTTP_CHS_WINDOWS_1255, "windows-1255" },
	{ HTTP_CHS_WINDOWS_1256, "windows-1256" },
	{ HTTP_CHS_WINDOWS_1257, "windows-1257" },
	{ HTTP_CHS_WINDOWS_1258, "windows-1258" },
	{ HTTP_CHS_WINDOWS_874, "windows-874" },
};

const LookupEntry http_connection_lookup_table_entries[HTTP_CONNECTION_TABLE_COUNT] = {
	{ HTTP_CON_KEEP_ALIVE, "keep-alive" },
	{ HTTP_CON_CLOSE, "close" },
	{ HTTP_CON_UPGRADE, "upgrade" },
};

const LookupEntry http_req_cache_control_lookup_table_entries[HTTP_REQ_CACHE_CONTROL_TABLE_COUNT] = {
	{ HTTP_REQ_CC_MAX_AGE, "max-age" },
	{ HTTP_REQ_CC_MAX_STALE, "max-stale" },
	{ HTTP_REQ_CC_MIN_FRESH, "min-fresh" },
	{ HTTP_REQ_CC_NO_CACHE, "no-cache" },
	{ HTTP_REQ_CC_NO_STORE, "no-store" },
	{ HTTP_REQ_CC_NO_TRANSFORM, "no-transform" },
	{ HTTP_REQ_CC_ONLY_IF_CACHED, "only-if-cached" },
	{ HTTP_REQ_CC_STALE_IF_ERROR, "stale-if-error" },
};

const LookupEntry http_res_cache_control_lookup_table_entries[HTTP_RES_CACHE_CONTROL_TABLE_COUNT] = {
	{ HTTP_RES_CC_MAX_AGE, "max-age" },
	{ HTTP_RES_CC_S_MAX_AGE, "s-maxage" },
	{ HTTP_RES_CC_NO_CACHE, "no-cache" },
	{ HTTP_RES_CC_NO_STORE, "no-store" },
	{ HTTP_RES_CC_NO_TRANSFORM, "no-transform" },
	{ HTTP_RES_CC_MUST_REVALIDATE, "must-revalidate" },
	{ HTTP_RES_CC_PROXY_REVALIDATE, "proxy-revalidate" },
	{ HTTP_RES_CC_MUST_UNDERSTAND, "must-understand" },
	{ HTTP_RES_CC_PRIVATE, "private" },
	{ HTTP_RES_CC_PUBLIC, "public" },
	{ HTTP_RES_CC_IMMUTABLE, "immutable" },
	{ HTTP_RES_CC_STALE_WHILE_REVALIDATE, "stale-while-revalidate" },
	{ HTTP_RES_CC_STALE_IF_ERROR, "stale-if-error" },
};

const LookupEntry day_lookup_table_entries[DAY_TABLE_COUNT] = {
	{ DAY_MON, "Mon" },
	{ DAY_TUE, "Tue" },
	{ DAY_WED, "Wed" },
	{ DAY_THU, "Thu" },
	{ DAY_FRI, "Fri" },
	{ DAY_SAT, "Sat" },
	{ DAY_SUN, "Sun" },
};

const LookupEntry month_lookup_table_entries[MONTH_TABLE_COUNT] = {
	{ MONTH_JAN, "Jan" },
	{ MONTH_FEB, "Feb" },
	{ MONTH_MAR, "Mar" },
	{ MONTH_APR, "Apr" },
	{ MONTH_MAY, "May" },
	{ MONTH_JUN, "Jun" },
	{ MONTH_JUL, "Jul" },
	{ MONTH_AUG, "Aug" },
	{ MONTH_SEP, "Sept" },
	{ MONTH_OCT, "Oct" },
	{ MONTH_NOV, "Nov" },
	{ MONTH_DEC, "Dec" },
};

const LookupEntry http_req_upgrade_lookup_table_entries[HTTP_REQ_UPGRADE_TABLE_COUNT] = {
	{ HTTP_UPG_WS, "websocket" },
};

const LookupEntry ws_close_code_lookup_table_entries[WS_CLOSE_CODE_TABLE_COUNT] = {
	{ WS_CC_1000, "Normal Closure" },
	{ WS_CC_1001, "Going Away" },
	{ WS_CC_1002, "Protocol Error" },
	{ WS_CC_1003, "Unsupported Data" },
	{ WS_CC_1005, "No Status Received" },
	{ WS_CC_1006, "Abnormal Closure" },
	{ WS_CC_1007, "Invalid frame payload data" },
	{ WS_CC_1008, "Policy Violation" },
	{ WS_CC_1009, "Message too big" },
	{ WS_CC_1010, "Extension Required" },
	{ WS_CC_1011, "Internal Error" },
	{ WS_CC_1015, "TLS Handshake" },
};

const LookupEntry http_content_encoding_lookup_table_entries[HTTP_CONTENT_ENCODING_TABLE_COUNT] = {
	{ HTTP_CONENC_GZIP, "gzip" },
	{ HTTP_CONENC_COMPRESS, "compress" },
	{ HTTP_CONENC_DEFLATE, "deflate" },
	{ HTTP_CONENC_BR, "br" },
	{ HTTP_CONENC_ZSTD, "zstd" },
	{ HTTP_CONENC_DCB, "dcb" },
	{ HTTP_CONENC_DCZ, "dcz" },
};

/*****************************************************************************/

const LookupTable http_method_lookup_table = {
	.entries = http_method_lookup_table_entries,
	.size = HTTP_METHOD_TABLE_COUNT
};

const LookupTable http_req_header_field_lookup_table = {
	.entries = http_req_header_field_lookup_table_entries,
	.size = HTTP_REQ_HEADER_FIELD_TABLE_COUNT
};

const LookupTable http_status_code_lookup_table = {
	.entries = http_status_code_lookup_table_entries,
	.size = HTTP_STATUS_CODE_TABLE_COUNT
};

const LookupTable http_res_header_field_lookup_table = {
	.entries = http_res_header_field_lookup_table_entries,
	.size = HTTP_RES_HEADER_FIELD_TABLE_COUNT
};

const LookupTable http_media_type_lookup_table = {
	.entries = http_media_type_lookup_table_entries,
	.size = HTTP_MEDIA_TYPE_TABLE_COUNT
};

const LookupTable http_accept_encoding_lookup_table = {
	.entries = http_accept_encoding_lookup_table_entries,
	.size = HTTP_ACCEPT_ENCODING_TABLE_COUNT
};

const LookupTable http_te_lookup_table = {
	.entries = http_te_lookup_table_entries,
	.size = HTTP_TE_TABLE_COUNT
};

const LookupTable http_transfer_encoding_lookup_table = {
	.entries = http_transfer_encoding_lookup_table_entries,
	.size = HTTP_TRANSFER_ENCODING_TABLE_COUNT
};

const LookupTable http_charset_lookup_table = {
	.entries = http_charset_lookup_table_entries,
	.size = HTTP_CHARSET_TABLE_COUNT
};

const LookupTable http_connection_lookup_table = {
	.entries = http_connection_lookup_table_entries,
	.size = HTTP_CONNECTION_TABLE_COUNT
};

const LookupTable http_req_cache_control_lookup_table = {
	.entries = http_req_cache_control_lookup_table_entries,
	.size = HTTP_REQ_CACHE_CONTROL_TABLE_COUNT
};

const LookupTable http_res_cache_control_lookup_table = {
	.entries = http_res_cache_control_lookup_table_entries,
	.size = HTTP_RES_CACHE_CONTROL_TABLE_COUNT
};

const LookupTable day_lookup_table = {
	.entries = day_lookup_table_entries,
	.size = DAY_TABLE_COUNT
};

const LookupTable month_lookup_table = {
	.entries = month_lookup_table_entries,
	.size = MONTH_TABLE_COUNT
};

const LookupTable http_req_upgrade_lookup_table = {
	.entries = http_req_upgrade_lookup_table_entries,
	.size = HTTP_REQ_UPGRADE_TABLE_COUNT
};

const LookupTable ws_close_code_lookup_table = {
	.entries = ws_close_code_lookup_table_entries,
	.size = WS_CLOSE_CODE_TABLE_COUNT
};

const LookupTable http_content_encoding_lookup_table = {
	.entries = http_content_encoding_lookup_table_entries,
	.size = HTTP_CONTENT_ENCODING_TABLE_COUNT
};

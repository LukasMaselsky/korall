#include "server.h"
#include "main.h"

static void my_route(HTTPRequest* req, HTTPResponse* res) {
	res->start_line->status_code = HTTP_SC_401;
}

int main(int argc, char* argv[]) {

	ServerConfig config = {
		.domain = NULL,
		.port = "3500",
		.name = "CustomServer",
		.type = ST_HTTP
	};

	const Route route_arr[1] = {
		{.path = "/", .method = HTTP_GET, .callback = my_route }
	};
	const Routes routes = { .routes = route_arr, .route_count = 1 };

	http_server_run(&config, &routes);

	return 0;
}

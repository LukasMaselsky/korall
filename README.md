# korall

HTTP and WebSocket server library in C.

## Table of Contents

- [Features](#features)
- [Dependencies](#dependencies)
- [Installation](#installation)
	- [Installing OpenSSL](#installing-openssl)
- [Build](#build)
	- [Docker](#docker)	
- [Examples](#examples)
- [Tests](#tests)
- [API](#api)
	- [Config](#config)

## Features

- HTTP and HTTPS support
- HTTP header and format parsing
- HTTP route and query parameter handling
- WebSocket support
- Connection management using threads

## Dependencies

- Standard C toolchain
- cJSON
- OpenSSL

## Installation

### Installing OpenSSL

#### Windows

OpenSSL can be installed through the [SLP](https://slproweb.com/products/Win32OpenSSL.html?ref=passwork.pro/blog) installer.

NOTE: the **full** version, not the light version, of OpenSSL should be installed.

#### Linux

Use your appropriate package manager to install OpenSSL.

On Debian/Ubuntu systems this would be:

```sh
sudo apt-get install libssl-dev
```

On Red Hat systems like Fedora this would be:

```sh
sudo dnf install openssl-devel
```

## Build

1. Clone repo

	```sh
	git clone https://github.com/LukasMaselsky/korall.git
	```


2. CMake

	```sh
	mkdir out
	cd out
	cmake ..
	```

## Docker

1. Build image
	
	```sh
	docker build -t korall .
	```

2. Run container 

	```sh
	docker run -it -p 3500:3500 korall
	```

## Examples

For examples, see the ```examples/``` directory.

## Tests

For tests, see the ```tests/``` directory.

## API

### Config

| Name              | Type               | Description                                          | Default      |
| ----------------- | ------------------ | ---------------------------------------------------- | ------------ |
| domain            | ```String```       | Domain URL                                           | localhost    |
| port              | ```String```       | Port number                                          | 3500         |
| name              | ```String```       | Server name                                          | KorallServer |
| allow_origins     | ```Array*```       | ```char**``` dynamic array of allowed origins        | \*           |
| allow_headers     | ```Array*```       | ```char**``` dynamic array of allowed custom headers | \*           |
| allow_methods     | ```Array*```       | ```ìnt``` dynamic array of allowed methods           | \*           |
| allow_credentials | ```boolean```      | Allow credentials                                    | false        |
| secure            | ```boolean```      | Use HTTPS                                            | true         |
| max_http_routes   | ```unsigned int``` | Maximum number of HTTP routes capacity               | 100          |
| max_ws_routes     | ```unsigned int``` | Maximum number of WebSocket routes capacity          | 100          |
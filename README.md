# korall

HTTP web server library in C.

## Table of Contents

- [Features](#features)
- [Prerequisites](#prerequisites)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Build](#build)
- [Examples](#examples)
- [Tests](#tests)
- [API](#api)
	- [Config](#config)

## Features

## Prerequisites

- CMake

## Dependencies

- cJSON
- OpenSSL

## Installation

1. Clone repo

	```sh
	git clone --recurse-submodules https://github.com/LukasMaselsky/korall.git
	```

2. vcpkg installation

	```sh
	./vcpkg/bootstrap-vcpkg.bat
	# or
	./vcpkg/bootstrap-vcpkg.sh
	```
3. CMake

	```sh
	mkdir out
	cd out
	cmake ..
	```

## Build

### CMake

```bash
mkdir build
cd build
cmake ..
make
```

### Make

```bash
make
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
| max_http_routes   | ```unsigned int``` | Maximum number of HTTP routes capacity               | 100          |
| max_ws_routes     | ```unsigned int``` | Maximum number of WebSocket routes capacity          | 100          |

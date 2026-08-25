# korall

HTTP and WebSocket server library in C.

## Table of Contents

- [Features](#features)
- [Dependencies](#dependencies)
- [Installation](#installation)
	- [Installing OpenSSL](#installing-openssl)
- [Build](#build)
- [Examples](#examples)
- [Tests](#tests)
- [API](#api)
	- [Config](#config)

## Features

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

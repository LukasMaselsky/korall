all:
	make -C tests
	make -C examples/http
	make -C examples/echo

clean:
	make -C tests clean
	make -C examples/http clean
	make -C examples/echo clean
all:
	make -C korall
	make -C examples/http
	make -C examples/echo

clean:
	make -C korall clean
	make -C examples/http clean
	make -C examples/echo clean
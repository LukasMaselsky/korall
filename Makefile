BINARY=bin
CODEDIRS=korall/src korall/tests
INCDIRS=./korall/include/korall/ ./korall/src/


CC=gcc
OPT=-O0
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -g $(foreach D, $(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

CFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.c))
OBJECTS=$(patsubst %.c,%.o,$(CFILES))
DEPFILES=$(patsubst %.c,%.d,$(CFILES))


all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY) $(OBJECTS) $(DEPFILES)

-include $(DEPFILES)
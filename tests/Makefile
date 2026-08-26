BINARY=bin
CODEDIRS=../src ../deps .
INCDIRS=../include ../src ../deps
LIBRARIES=ssl crypto

CC=gcc
OPT=-O0
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -g $(foreach D, $(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
LIBLINKS=$(foreach L, $(LIBRARIES),-l$(L))
CFILES=$(call rwildcard,$(CODEDIRS),*.c)
OBJECTS=$(patsubst %.c,%.o,$(CFILES))
DEPFILES=$(patsubst %.c,%.d,$(CFILES))


all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) $(LIBLINKS) -o $@ $^

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY) $(OBJECTS) $(DEPFILES)

-include $(DEPFILES)
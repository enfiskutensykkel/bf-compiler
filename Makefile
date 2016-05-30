PROJECT := bfc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic -DDATA_ADDR=0x1000000000 -DTEXT_ADDR=0x1000010000 -DPAGE_SIZE=0x1000 -DMAX_NESTED_LOOPS=0x1000
CC	:= clang

SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)

OBJECTS := $(SOURCES:%.c=%.o)


.PHONY: bin/$(PROJECT) all clean debug

all: bin/$(PROJECT)

clean:
	-$(RM) $(PROJECT) $(OBJECTS)

bin/$(PROJECT): $(OBJECTS)
	-mkdir -p $(@D)
	$(CC) -o $@ $^

debug: CFLAGS += -DDEBUG -g
debug: $(PROJECT)

%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $<


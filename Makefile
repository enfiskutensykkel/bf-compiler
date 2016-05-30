PROJECT := bfc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic -DDATA_ADDR=0x1000000000 -DTEXT_ADDR=0x1000010000 -DPAGE_SIZE=0x1000 
CC	:= clang

SOURCES := $(wildcard src/*.c)
HEADERS := $(wildcard src/*.h)

OBJECTS := $(SOURCES:%.c=%.o)

.PHONY: $(PROJECT) all clean debug

all: $(PROJECT)

clean:
	-$(RM) $(PROJECT) $(OBJECTS)

$(PROJECT): $(OBJECTS)
	$(CC) -o $@ $^

debug: CFLAGS += -DDEBUG -g
debug: $(PROJECT)

%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $<


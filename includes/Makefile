CC      := cc
CFLAGS  := -Wall -Werror -Wextra -Wpedantic -O2
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

all: $(OBJECTS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm *.o

TARGET  := solve.out
CC      := cc
CFLAGS  := -Wall -Werror -Wpedantic -Wextra -O2
OBJECTS := $(wildcard ../../includes/*.o)

TARGET  := solve.out
all: $(TARGET)

$(TARGET): ./main.c
	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^ -lm

clean:
	rm $(TARGET)
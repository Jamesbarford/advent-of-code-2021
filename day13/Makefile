TARGET  := solve.out
CC      := cc
CFLAGS  := -Wall -Wpedantic -Wextra -O0 -g
OBJECTS := $(wildcard ../includes/*.o)

TARGET  := solve.out
all: $(TARGET)

$(TARGET): ./main.c
	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGET)
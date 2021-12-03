TARGET  := solve.out
ALTERNATIVE := alt.out
CC      := cc
CFLAGS  := -Wall -Wpedantic -Wextra -O2
OBJECTS := $(wildcard ../../includes/*.o)
HEADERS := $(wildcard ../../includes/*.h)

all: $(TARGET) $(ALTERNATIVE)

$(TARGET): ./main.c
	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^

$(ALTERNATIVE): ./alternative.c
	$(CC) $(OBJECTS) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGET)

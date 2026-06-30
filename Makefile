CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
TARGET=proj2

all: $(TARGET)

$(TARGET): proj2.c
	$(CC) $(CFLAGS) proj2.c -o $(TARGET)

clean:
	rm -f $(TARGET)
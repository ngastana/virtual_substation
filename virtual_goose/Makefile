CC = gcc
CFLAGS = -Wall -g -Iinclude

# Lista de archivos fuente
SRCS = src/virtual_goose.c goose_publisher.c
OBJS = $(SRCS:.c=.o)
TARGET = virtual_goose

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

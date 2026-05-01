CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude
COMMON  = src/common.c

all: server client

server: src/server.c $(COMMON)
	$(CC) $(CFLAGS) -lpthread -o $@ $^

client: src/client.c $(COMMON)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f server client

.PHONY: all clean

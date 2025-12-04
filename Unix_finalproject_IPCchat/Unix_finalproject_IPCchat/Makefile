CC = gcc
CFLAGS = -I./include -Wall
TARGETS = bin/server bin/client

all: $(TARGETS)

bin/server: src/server.c src/utils.c
	$(CC) $(CFLAGS) -o $@ $^

bin/client: src/client.c src/utils.c src/file_handler.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f bin/* /tmp/*_fifo
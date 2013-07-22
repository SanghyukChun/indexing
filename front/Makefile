.PHONY: clean

all: client server

client: client.c
	gcc -o $@ $^

server: server.c
	gcc -o $@ $^

clean:
	rm server client *.o *~
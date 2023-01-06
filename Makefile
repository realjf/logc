package = logc
version = 0.1.0

CC=gcc
CFLAGS=-I.
BIN=./bin/test

test: ./test/logc.c
	mkdir -p bin && $(CC) $(CFLAGS) -o $(BIN) ./test/logc.c

clean:
	rm -rf $(BIN)

push:
	@git add -A && git commit -m "update" && git push origin master


.PHONY: test clean push

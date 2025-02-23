CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c17
all: main

main: main.c
	$(CC) -o ack-tuah main.c $(CFLAGS)

clean:
	rm ack-tuah


CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c17 -ggdb
INCLUDE=-I ./include
all: ack-tuah

ack-tuah: src/main.c src/tcp.c src/ipv4.c
	$(CC) -o ack-tuah src/main.c src/tcp.c src/ipv4.c $(CFLAGS) $(INCLUDE)

run: ack-tuah
	sudo ./ack-tuah

clean:
	rm ack-tuah



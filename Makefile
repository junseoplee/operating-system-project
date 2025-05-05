CC=gcc
CFLAGS=-Wall -O2 -lpthread

all: compressor

compressor: main.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main.c compress.h
	$(CC) -c main.c

compress.o: compress.c compress.h
	$(CC) -c compress.c

clean:
	rm -f *.o compressor

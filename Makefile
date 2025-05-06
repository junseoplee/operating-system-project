CC=gcc
CFLAGS=-Wall -O2 -lpthread

all: compressor compressor_threaded

compressor: main.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

compressor_threaded: main_threaded.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main.c compress.h
	$(CC) -c main.c

main_threaded.o: main_threaded.c compress.h
	$(CC) -c main_threaded.c

compress.o: compress.c compress.h
	$(CC) -c compress.c

clean:
	rm -f *.o compressor compressor_threaded

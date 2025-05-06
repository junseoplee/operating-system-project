CC=gcc
CFLAGS=-Wall -O2 -lpthread

all: compressor compressor_mutex compressor_semaphore

compressor: main.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

compressor_mutex: main_mutex.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

compressor_semaphore: main_semaphore.o compress.o
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main_mutex.c compress.h
	$(CC) -c main.c

main_mutex.o: main_mutex.c compress.h
	$(CC) -c main_mutex.c

main_semaphore.o: main_semaphore.c compress.h
	$(CC) -c main_semaphore.c

compress.o: compress.c compress.h
	$(CC) -c compress.c

clean:
	rm -f *.o compressor compressor_mutex compressor_semaphore

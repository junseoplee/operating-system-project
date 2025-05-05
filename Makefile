CC=gcc
CFLAGS=-Wall -O2 -lpthread
OBJ=main.o compress.o

all: compressor

compressor: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

main.o: main.c compress.h
	$(CC) -c main.c

compress.o: compress.c compress.h
	$(CC) -c compress.c

clean:
	rm -f *.o compressor

CC=cc
CFLAGS=-Wall -O2

sieben: sieben.o
	$(CC) sieben.o -lpthread -o sieben

sieben.o: sieben.c
	$(CC) $(CFLAGS) -c sieben.c


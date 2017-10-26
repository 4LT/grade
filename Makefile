CC = cc
DBGFLAGS = -ggdb
CFLAGS = $(DBGFLAGS) -pedantic -Wall
LD = $(CC)
LDFLAGS = 

.PHONY = all clean

all: Makefile main.o
	$(LD) main.o -o grade $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

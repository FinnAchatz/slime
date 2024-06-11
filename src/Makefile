CC = gcc
#CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O3
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g

all: slime

%.o: %.c
	$(CC) $(CFLAGS) -c $@ $< -lm

slime: simulate.c gifenc.o gifenc.h simulate.o
	$(CC) $(CFLAGS) -o slime gifenc.o simulate.c -lm

clean:
	rm slime *.o


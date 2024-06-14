CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O3 -ffast-math
# CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -O3 -ffast-math -g -pg
# CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g

all: slime

%.o: %.c
	$(CC) $(CFLAGS) -c $@ $< -lm

slime: simulate.c gifenc.o random.o framework.o agent.o simulate.o cgif.o cgif_raw.o
	$(CC) $(CFLAGS) -o slime simulate.c gifenc.o random.o framework.o agent.o cgif.o cgif_raw.o -lm

clean:
	rm slime *.o


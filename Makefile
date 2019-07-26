CFLAGS=`pkg-config --cflags glib-2.0` -std=c99 -Wall -O3 -march=native -c
LDFLAGS=`pkg-config --cflags --libs glib-2.0`
CC=gcc

all: tictactoe

tictactoe: tictac.o
	$(CC) tictac.o $(LDFLAGS) -o tictactoe

tictac.o: tictac.c
	$(CC) $(CFLAGS) tictac.c

clean:
	rm *.o


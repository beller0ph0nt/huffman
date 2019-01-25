CC = g++
CFLAGS = -ggdb -Wall -std=c++14

.PHONY: all clean

all: encoder decoder

encoder: common.o encoder.o
	$(CC) -o $@ $^

decoder: common.o decoder.o
	$(CC) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $^

clean:
	rm -f encoder decoder *.o


CC=g++
CFLAGS=-c
LIBS=-lboost_system
VPATH=./src
OBJECTS = getR2.o InOut.o Utils.o

all: main

main: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o getr2

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


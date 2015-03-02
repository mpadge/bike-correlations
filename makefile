CC=g++
CFLAGS=-c
LIBS=-lzip
VPATH=./src
OBJECTS = main.o StationData.o RideData.o Utils.o

all: main

main: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o getr2

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


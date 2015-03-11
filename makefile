CC=g++
CFLAGS=-c -std=c++11
LIBS=-lzip
VPATH=./src
OBJECTS = main.o StationData.o RideData.o Utils.o

all: main

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o bikes $(LIBS) 

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


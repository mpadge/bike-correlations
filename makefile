CC=g++
CFLAGS=-c
LIBS=-lzip
VPATH=./src
OBJECTS = main.o StationData.o RideData.o Utils.o

all: main

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o getr2 $(LIBS) 

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


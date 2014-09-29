CC=g++
CFLAGS=-c
LIBS=-I /usr/include/boost_1_54_0
VPATH=./src
OBJECTS = getR2.o InOut.o Calculations.o Utils.o

all: main

main: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o getr2

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


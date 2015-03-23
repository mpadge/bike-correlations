CC=g++
CFLAGS=-c -std=c++11
LIBS=-lzip
VPATH=./src
OBJECTS_BIKES = mainBikes.o StationData.o RideData.o Utils.o
OBJECTS_TRAINS = mainTrains.o StationData.o TrainData.o Utils.o

bikes: mainBikes

mainBikes: $(OBJECTS_BIKES)
	$(CC) $(OBJECTS_BIKES) -o bikes $(LIBS) 

trains: mainTrains

mainTrains: $(OBJECTS_TRAINS)
	$(CC) $(OBJECTS_TRAINS) -o trains $(LIBS) 

%.o: %.c++
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o 


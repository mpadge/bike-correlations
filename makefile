#CC=g++
CFLAGS=-c -std=c++11
LIBS=-lzip 
VPATH=./src
OBJECTS_BIKES = mainBikes.o StationData.o RideData.o Utils.o
OBJECTS_TRAINS = mainTrains.o StationData.o TrainData.o Utils.o
OBJECTS_DISTS = mainStnDists.o Utils.o

all: mainBikes mainTrains mainStnDists

bikes: mainBikes

mainBikes: $(OBJECTS_BIKES)
	$(CXX) $(OBJECTS_BIKES) -o bikes $(LIBS) 

trains: mainTrains

mainTrains: $(OBJECTS_TRAINS)
	$(CXX) $(OBJECTS_TRAINS) -o trains $(LIBS) 

dists: mainStnDists

mainStnDists: $(OBJECTS_DISTS)
	$(CXX) $(OBJECTS_DISTS) -o dists $(LIBS) 

%.o: %.c++
	$(CXX) $(CFLAGS) $<

clean:
	rm -f *.o 


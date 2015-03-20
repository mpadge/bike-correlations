#include "mainTrains.h"

int main (int argc, char *argv[]) {

    TrainData trainData;
    std::cout << "There are " << trainData.getNumRailStations () << 
        " rail and " << trainData.getNumTubeStations () << 
        " tube stations." << std::endl;
}

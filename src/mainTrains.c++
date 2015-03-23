#include "mainTrains.h"

int main (int argc, char *argv[]) {

    TrainData railData ("OysterRail", false),
              tubeData ("OysterTube", true);
    std::cout << "There are " << railData.returnNumStations () << 
        " rail and " << tubeData.returnNumStations () << 
        " tube stations." << std::endl;
    railData.getTrainStations();
}

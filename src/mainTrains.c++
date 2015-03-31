#include "mainTrains.h"

int main (int argc, char *argv[]) {

    TrainData tubeData ("oysterTube", true);
    std::cout << "There are " << tubeData.CountTrips () <<
        " trips between " << tubeData.Oyster2StnIndex.size () << " stations." <<
        std::endl;

    TrainData railData ("oysterRail", false);
    std::cout << "There are " << railData.CountTrips () <<
        " trips between " << railData.Oyster2StnIndex.size () << " stations." <<
        std::endl;
}

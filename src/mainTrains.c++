#include "mainTrains.h"

int main (int argc, char *argv[]) {

    //TrainData railData ("oysterTube", true);
    TrainData railData ("oysterRail", false);
    std::cout << "There are " << railData.CountTrips () <<
        " trips between " << railData.Oyster2StnIndex.size () << " stations." <<
        std::endl;
}

#include "mainTrains.h"

int main (int argc, char *argv[]) {

    TrainData railData ("OysterRail", false),
              tubeData ("OysterTube", true);
    for (std::vector <StationData::OneStation>::iterator
            itr=railData.StationList.begin(); 
            itr != railData.StationList.end(); itr++)
    {
        //std::cout << "<" << standardise ((*itr).name) << ">" << std::endl;
    }
    std::cout << "There are " << railData.returnNumStations () << 
        " rail and " << tubeData.returnNumStations () << 
        " tube stations." << std::endl;
    railData.getTrainData (false);
}

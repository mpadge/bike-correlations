#include "mainTrains.h"

int main (int argc, char *argv[]) {

    TrainData railData ("oysterTube", true);
    //TrainData railData ("oysterRail", false);
    for (std::vector <StationData::OneStation>::iterator
            itr=railData.StationList.begin(); 
            itr != railData.StationList.end(); itr++)
    {
        //std::cout << "<" << (*itr).name << ">" << std::endl;
    }
    std::cout << "There are " << railData.returnNumStations () << 
        " stations." << std::endl;
    //railData.getTrainData (false);
    //railData.getTrainData (true);
}

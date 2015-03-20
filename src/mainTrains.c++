#include "mainTrains.h"

int main(int argc, char *argv[]) {
    int nfiles, count, tempi [2];
    std::vector <std::string> tempstr;
    std::string city, nfext;

    std::cout << std::endl << "_____________________________________________" << 
        "____________________________________________" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t./bikes with the following 3 parameters " <<
        "(defaulting to first values):\t\t|" << std::endl;
    std::cout << "|\t1. <city> for <city>=<london/nyc/oyster>\t\t\t\t\t|" << std::endl;
    std::cout << "|\t2. (0,1,2) for analyses of (all, subscriber, customer)" <<
        " data (NYC only)\t\t|" << std::endl;
    std::cout << "|\t ---or set the second parameter to >2 for analysis of" <<
        " age-class data---\t\t|" << std::endl;
    std::cout << "|\t ---in which case the 3rd parameter is the decade to"
        " be analysed---\t\t|" << std::endl;
    std::cout << "|\t ---or set to 0 for \"young\" = <40, and 1 for \"old\"" <<
        "---\t\t\t\t|" << std::endl;
    std::cout << "|\t3. (0,1,2) for analyses of (all, male, female) " <<
        "data (NYC only)\t\t\t|" << std::endl;
    std::cout << "|\t(NOTE that (male,female) can only be analysed for " <<
        "subscribers.)\t\t\t|" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\tCalculations loop over the two conditions of:\t\t\t\t\t|" <<
        std::endl;
    std::cout << "|\t1.(include, ignore) stations with zero trips\t\t\t\t\t|" << 
        std::endl;
    std::cout << "|\t2. analyses of (all, near, far) data\t\t\t\t\t\t|" << 
        std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;

    TrainData trainData ();
    //tempi [0] = trainData.getTrainStations ();
    //tempi [0] = trainData.getTrainTrips ();
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;
}

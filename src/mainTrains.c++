#include "mainTrains.h"

int main (int argc, char *argv[]) {
    std::string r2name, covname;

    // **********   RAIL DATA   **********
    TrainData railData ("oysterRail", false);
    std::cout << "There are " << railData.CountTrips () <<
        " trips between " << railData.Oyster2StnIndex.size () << " stations." <<
        std::endl;
    railData.nearfar = 0; // important! Uses all data

    r2name = "R2_london_rail_from_all.csv";
    covname = "Cov_london_rail_from_all.csv";
    railData.calcR2 (true);
    railData.writeR2Mat (r2name);
    railData.writeCovMat (covname);

    r2name = "R2_london_rail_to_all.csv";
    covname = "Cov_london_rail_to_all.csv";
    railData.calcR2 (false);
    railData.writeR2Mat (r2name);
    railData.writeCovMat (covname);

    // **********   TUBE DATA   **********
    TrainData tubeData ("oysterTube", true);
    std::cout << "There are " << tubeData.CountTrips () <<
        " trips between " << tubeData.Oyster2StnIndex.size () << " stations." <<
        std::endl;

    tubeData.nearfar = 0; // important! Uses all data

    r2name = "R2_london_tube_from_all.csv";
    covname = "Cov_london_tube_from_all.csv";
    tubeData.calcR2 (true);
    tubeData.writeR2Mat (r2name);
    tubeData.writeCovMat (covname);

    r2name = "R2_london_tube_to_all.csv";
    covname = "Cov_london_tube_to_all.csv";
    tubeData.calcR2 (false);
    tubeData.writeR2Mat (r2name);
    tubeData.writeCovMat (covname);
}

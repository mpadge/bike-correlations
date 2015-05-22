/***************************************************************************
 * This software is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * <mainTrains.c++>
 *
 * Author: Mark Padgham, May 2015
 ***************************************************************************/

#include "mainTrains.h"

int main (int argc, char *argv[]) {
    std::string r2name, covname;

    std::cout << std::endl << "_____________________________________________" << 
        "____________________________________________" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t./trains\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\tCalculations loop over analyses of " <<
        "(all, near, far) data\t\t\t|" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;

    std::vector <std::string> mode = {"oysterRail", "oysterTube"};
    std::vector <std::string> tubetxt = {"rail", "tube"};
    std::vector <bool> tube = {false, true};

    for (int t=0; t<2; t++)
    {
        TrainData * trainData = new TrainData (mode [t], tube [t]);
        std::cout << "There are " << trainData->CountTrips () <<
            " trips between " << trainData->Oyster2StnIndex.size () << 
            " " << mode [t] << " stations." << std::endl;
        for (int i=0; i<3; i++)
        {
            trainData->nearfar = i;
            trainData->txtnf = trainData->txtnflist [i];

            r2name = "R2_london_" + tubetxt [t] + "_from_" + 
                trainData->txtnf + ".csv";
            covname = "Cov_london_" + tubetxt [t] + "_from_" + 
                trainData->txtnf + ".csv";
            trainData->calcR2 (true);
            trainData->writeR2Mat (r2name);
            trainData->writeCovMat (covname);

            r2name = "R2_london_" + tubetxt [t] + "_to_" + 
                trainData->txtnf + ".csv";
            covname = "Cov_london_" + tubetxt [t] + "_to_" + 
                trainData->txtnf + ".csv";
            trainData->calcR2 (false);
            trainData->writeR2Mat (r2name);
            trainData->writeCovMat (covname);
        }
    }
}

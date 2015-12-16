/***************************************************************************
 *  Project:    bike-correlations
 *  File:       mainTrains.c++
 *  Language:   C++
 *
 *  bike-correlations is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  bike-correlations is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  NeutralClusters.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright   Mark Padgham December 2015
 *  Author:     Mark Padgham
 *  E-Mail:     mark.padgham@email.com
 *
 *  Description:    Constructs correltaion matrices between all stations of
 *                  public bicycle hire systems for London, UK, and Boston,
 *                  Chicago, Washington DC, and New York, USA. Also analyses
 *                  Oystercard data for London.
 *
 *  Limitations:
 *
 *  Dependencies:       libboost
 *
 *  Compiler Options:   -std=c++11 -lboost_program_options -lzip
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

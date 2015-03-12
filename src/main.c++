#include "main.h"

int main(int argc, char *argv[]) {
    int nfiles, count, tempi [2];
    std::vector <std::string> tempstr;
    std::string city, nfext;

    std::cout << std::endl << "_____________________________________________" << 
        "____________________________________________" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t./bikes with the following 3 parameters " <<
        "(defaulting to first values):\t\t|" << std::endl;
    std::cout << "|\t1. <city> for <city>=<london/nyc>\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t2. (0,1,2) for analyses of (all, subscriber, customer)" <<
        " data (NYC only)\t\t|" << std::endl;
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

    city = "nyc";
    tempi [0] = tempi [1] = 0;
    count = -1;
    while (*++argv != NULL)
    {
        if (count < 0)
        {
            city = *argv;
            std::transform (city.begin(), city.end(), city.begin(), ::tolower);
            if (city.substr (0, 2) == "lo") {
                city = "london";
            } else {
                city = "nyc";
            }
        } else {
            tempi [count] = atoi (*argv);
            if (tempi [count] < 0 || tempi [count] > 2)
                tempi [count] = 0; 
        }
        count++;
    }
    RideData rideData (city, tempi [0], tempi [1]);

    std::cout << "|\t\tcity = " << city;
    if (city == "london")
        std::cout << "\t\t\t\t\t\t|" << std::endl;
    else {
        std::cout << " --- data = (";
        tempstr.resize (0);
        tempstr.push_back ("all");
        tempstr.push_back ("subscriber");
        tempstr.push_back ("customer");
        std::cout << tempstr [tempi [0]] << ", ";
        tempstr.resize (0);
        tempstr.push_back ("all");
        tempstr.push_back ("male");
        tempstr.push_back ("female");
        std::cout << tempstr [tempi [1]] << ")";
        if (tempi [0] == 0 || (tempi [0] == 1 && tempi [1] == 0) ||
                (tempi [0] == 2 && tempi [1] < 2))
            std::cout << "\t";
        std::cout << "\t\t\t\t|" << std::endl;
        tempstr.resize (0);
    }
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;

    int numStations = rideData.getNumStations();
    std::cout << "There are " << numStations << 
        " stations [max#=" << rideData.getStnIndxLen() << "] and " << 
        rideData.getNumFiles() << " trip files." << std::endl;

    count = 0;
    if (city == "london") {
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            nfiles = rideData.countFilesLondon (i);
            for (int j=0; j<nfiles; j++) {
                tempi [0] = rideData.unzipOneFileLondon (i, j);
                std::cout << "Reading file[";
                if (j < 10)
                    std::cout << " ";
                std::cout << j << "/" << nfiles << "]:" << rideData.fileName;
                std::cout.flush ();
                tempi [0] = rideData.readOneFileLondon ();
                std::cout << " = " << tempi [0] << " trips." << std::endl;
                count += tempi [0];
                tempi [0] = rideData.removeFile ();
            }
            rideData.dumpMissingStations ();
            std::cout << "Total Number of Trips = " << count << std::endl;
        }
    } else { // city = NYC
        count = 0;
        for (int i=17; i<rideData.getNumFiles(); i++)
        {
            tempi [0] = rideData.getZipFileNameNYC (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileNYC (i);
                tempi [0] = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
        tempi [0] = rideData.aggregateTrips ();
    }

    rideData.writeDMat (); // Also fills RideData.dists
    rideData.writeNumTrips ();

    // Then loop over (include,ignore) zeros and (all, near, far) data
    for (int i=0; i<2; i++)
    {
        if (i == 0) rideData.ignoreZeros = false;
        else rideData.ignoreZeros = true;
        rideData.txtzero = rideData.txtzerolist [i];
        for (int j=0; j<3; j++)
        {
            rideData.nearfar = j;
            rideData.txtnf = rideData.txtnflist [j];

            rideData.calcR2 (true);
            rideData.writeR2Mat (true);
            rideData.writeCovMat (true);
            rideData.calcR2 (false);
            rideData.writeR2Mat (false);
            rideData.writeCovMat (false);
        }
    }
    //rideData.readR2Mat (false);
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;
}

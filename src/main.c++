#include "main.h"

int main(int argc, char *argv[]) {
    int nfiles, count, tempi [4];
    std::vector <std::string> tempstr;
    std::string city, nfext;

    std::cout << std::endl << "_____________________________________________" << 
        "____________________________________________" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t./bikes with the following 4 parameters " <<
        "(defaulting to first values):\t\t|" << std::endl;
    std::cout << "|\t1. <city> for <city>=<london/nyc>\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t2. (0,1) for (include, ignore) stations " <<
        "with zero trips\t\t\t\t|" << std::endl;
    std::cout << "|\t3. (0,1,2) for analyses of (all, near, far) data\t\t\t\t|" << 
        std::endl;
    std::cout << "|\t4. (0,1,2) for analyses of (all, subscriber, customer)" <<
        " data (NYC only)\t\t|" << std::endl;
    std::cout << "|\t5. (0,1,2) for analyses of (all, male, female) " <<
        "data (NYC only)\t\t\t|" << std::endl << 
        "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;

    city = "nyc";
    tempi [0] = tempi [1] = tempi [2] = tempi [3] = 0;
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
    std::cout << "|\tcity = " << city << " ---\t";
    bool ignoreZeros = false;
    if (tempi [0] > 0)
    {
        ignoreZeros = true;
        std::cout << "ignoring zeros ---\t";
    } else {
        std::cout << "not ignoring zeros ---\t";
    }
    tempstr.resize (0);
    tempstr.push_back ("all");
    tempstr.push_back ("near");
    tempstr.push_back ("far");
    std::cout << "data = (";
    std::cout << tempstr [tempi [1]];
    nfext = "_" + tempstr [tempi [1]];
    tempstr.resize (0);
    if (city == "nyc")
    {
        tempstr.push_back ("all");
        tempstr.push_back ("subscriber");
        tempstr.push_back ("customer");
        std::cout << ", " << tempstr [tempi [2]] << ", ";
        tempstr.resize (0);
        tempstr.push_back ("all");
        tempstr.push_back ("male");
        tempstr.push_back ("female");
        std::cout << tempstr [tempi [3]] << ")";
        if (tempi [2] == 0)
            std::cout << "\t";
        std::cout << "\t\t|" << std::endl;
        tempstr.resize (0);
    } else {
        std::cout << ")\t\t\t\t|" << std::endl;
    }
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;

    RideData rideData (city, ignoreZeros, tempi [0], tempi [1], tempi [2]);
    rideData.nfext = nfext;

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
    } else {
        count = 0;
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            tempi [0] = rideData.getZipFileNameNYC (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileNYC (i);
                tempi [0] = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
    }
    tempi [0] = rideData.aggregateTrips ();

    bool standardise = rideData.getStandardise ();

    rideData.writeDMat (); // Also fills RideData.dists
    rideData.writeNumTrips ();
    rideData.calcR2 (true);
    rideData.writeR2Mat (true);
    rideData.writeCovMat (true);
    rideData.calcR2 (false);
    rideData.writeR2Mat (false);
    rideData.writeCovMat (false);
    //rideData.readR2Mat (false);
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;
}

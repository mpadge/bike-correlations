#include "main.h"

int main(int argc, char *argv[]) {
    int nfiles, count = 0, tempi;
    bool from;
    ivec ntripRanges;
    std::vector <std::string> filelist, names;
    std::string city, fname;
    clock_t timer[2];

    timer[0] = clock();

    std::cout << "./getr2 1 <city> for <city>=<london/nyc>" << std::endl;
    if (argc > 1) {
        city = argv [2];
        std::transform (city.begin(), city.end(), city.begin(), ::tolower);
        if (city.substr (0, 2) == "lo") {
            city = "london";
        } else {
            city = "nyc";
        }
    } else {
        city = "london";
    }
    std::cout << "city = " << city << std::endl;

    RideData rideData (city);
    int numStations = rideData.getNumStations();
    std::cout << "There are " << numStations << 
        " stations [max#=" << rideData.getStnIndxLen() << "] and " << 
        rideData.getNumFiles() << " trip files." << std::endl;

    if (city == "london") {
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            nfiles = rideData.countFilesLondon (i);
            for (int j=0; j<nfiles; j++) {
                tempi = rideData.unzipOneFileLondon (i, j);
                std::cout << "Reading file[";
                if (j < 10)
                    std::cout << " ";
                std::cout << j << "/" << nfiles << "]:" << rideData.fileName;
                std::cout.flush ();
                tempi = rideData.readOneFileLondon ();
                std::cout << " = " << tempi << " trips." << std::endl;
                count += tempi;
                tempi = rideData.removeFile ();
            }
            rideData.dumpMissingStations ();
            std::cout << "Total Number of Trips = " << count << std::endl;
        }
    } else {
        count = 0;
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            // TODO: Clause to process .csv files directly without unzipping
            tempi = rideData.getZipFileNameNYC (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileNYC (i);
                tempi = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
    }
    rideData.writeNumTrips ();
    rideData.calcR2 (true);
    rideData.writeR2Mat (true);
    rideData.writeCovMat (true);
    rideData.calcR2 (false);
    rideData.writeR2Mat (false);
    rideData.writeCovMat (false);
    //rideData.readR2Mat (false);
    rideData.r2.resize (0,0);
    rideData.writeDMat ();
}

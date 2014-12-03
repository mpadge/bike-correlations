#include "getR2.h"

int main(int argc, char *argv[]) {
    int tempi;
    bool from;
    ivec ntripRanges;
    std::vector <std::string> filelist, names;
    std::string city, fname;
    clock_t timer[2];

    timer[0] = clock();

    std::cout << "./getr2 2 <city> 0 for dir=FROM (default) " <<
        "./getr2 2 <city> 1 for dir=TO" << 
        " for <city>=<london/nyc>" << std::endl;
    if (argc > 1) {
        city = argv [2];
        std::transform (city.begin(), city.end(), city.begin(), ::tolower);
        if (city.substr (0, 2) == "lo") {
            city = "london";
        } else {
            city = "nyc";
        }
        if (atoi (argv [3]) <= 0) {
            from = true;
            std::cout << city << ": Direction = FROM" << std::endl;
        }
        else {
            from = false;
            std::cout << city << ": Direction = TO" << std::endl;
        }
    } else {
        from = true;
        city = "london";
        std::cout << city << ": Direction = FROM" << std::endl;
    }

    filelist.resize (0);
    names.resize (0);
    getDir (&filelist);

    intPair stationIndex;
    tempi = getStationIndex (city, &stationIndex);
    int nstations = stationIndex.size ();
    std::cout << "There are " << nstations << " stations from " <<
        stationIndex.front().second << " to " << stationIndex.back().second <<
        std::endl;

    dvec lons, lats;
    lons.resize (nstations);
    lats.resize (nstations);
    names.resize (nstations);
    readLatLons (&lons, &lats, &stationIndex);
    getStationNames (&names);

    std::cout << "Loading data from " << filelist.size () << " files..." <<
        std::endl;
    imat ntrips = zmat_i (nstations, nstations);
    //filelist.resize (4); // for testing only
    for (int i=0; i<filelist.size (); i++) {
        std::cout << "[" << i << "]: ";
        tempi = readData (&ntrips, filelist [i]);
    }
    ntripRanges = tripNumRange (&ntrips);
    std::cout << "There are a total of " << ntripRanges (0) << 
        " trips connecting " << nstations - ntripRanges (2) << " / " <<
        nstations << " stations." << std::endl;

    dmat r2mat;
    r2mat.resize (ntrips.size1 (), ntrips.size2 ());
    dvec ranges = getCorrelations (&ntrips, &r2mat, from);
    writeR2mat (&r2mat, from);

    ntripRanges.resize (0);
    filelist.resize (0);
    lons.resize (0);
    lats.resize (0);
    names.resize (0);
    ntrips.resize (0, 0);
    r2mat.resize (0, 0);

    timer[1] = clock() - timer[0];
    std::cout<<"Total Calculation Time = ";
    timeout(timer[1] / ((double)CLOCKS_PER_SEC));
    std::cout<<std::endl;
}


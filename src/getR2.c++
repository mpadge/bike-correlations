#include "getR2.h"

int main () {
    int tempi;
    ivec ntripRanges;
    std::vector <std::string> filelist, names;
    std::string fname;
    clock_t timer[2];

    timer[0] = clock();

    filelist.resize (0);
    names.resize (0);
    getDir (&filelist);

    int nstations = getNumStations ();
    std::cout << "There are " << nstations << " stations." << std::endl;

    dvec lons, lats;
    lons.resize (nstations);
    lats.resize (nstations);
    names.resize (nstations);
    readLatLons (&lons, &lats);
    getStationNames (&names);

    std::cout << "Loading data from " << filelist.size () << " files..." <<
        std::endl;
    dmat ntrips = zmat_i (nstations, nstations);
    for (int i=0; i<filelist.size (); i++) {
        std::cout << "[" << i << "]: ";
        tempi = readData (&ntrips, filelist [i]);
    }
    ntripRanges = tripNumRange (&ntrips);
    std::cout << "There are a total of " << ntripRanges (0) << 
        " trips connecting " << nstations - ntripRanges (2) << " / " <<
        nstations << " stations." << std::endl;

    ntripRanges.resize (0);
    filelist.resize (0);
    lons.resize (0);
    lats.resize (0);
    names.resize (0);

    timer[1] = clock() - timer[0];
    std::cout<<"Total Calculation Time = ";
    timeout(timer[1] / ((double)CLOCKS_PER_SEC));
    std::cout<<std::endl;
}


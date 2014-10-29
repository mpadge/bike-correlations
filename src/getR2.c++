#include "getR2.h"

int main(int argc, char *argv[]) {
    int tempi;
    bool from;
    ivec ntripRanges;
    std::vector <std::string> filelist, names;
    std::string fname;
    clock_t timer[2];

    timer[0] = clock();

    std::cout << "./getr2 1 0 for dir=FROM (default) " <<
        "./getr2 1 1 for dir=TO" << std::endl;
    tempi = atoi (argv [2]);
    if (tempi <= 0) {
        from = true;
        std::cout << "Direction = FROM" << std::endl;
    }
    else {
        from = false;
        std::cout << "Direction = TO" << std::endl;
    }

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


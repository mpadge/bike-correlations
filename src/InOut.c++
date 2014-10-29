#include "InOut.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             GETDIR                                 **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void getDir (std::vector <std::string>* filelist)
{
    std::ifstream in_file;
    std::string dirtxt, fname;
    std::string configfile = "getr2.cfg"; // Contains name of data directory
    DIR *dir;
    struct dirent *ent;

    in_file.open (configfile.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        std::cout << "***ERROR: Write data directory in junk.cfg first!" << 
            std::endl;
    } else {
        getline (in_file, dirtxt, '\n');
        getline (in_file, dirtxt, '\n');
        if ((dir = opendir (dirtxt.c_str())) != NULL) {
            while ((ent = readdir (dir)) != NULL) {
                fname = ent->d_name;
                if (fname != "." && fname != "..") {
                    fname = dirtxt + '/' + fname;
                    (*filelist).push_back (fname);
                }
            }
            closedir (dir);
        } else {
            std::string outstr = "ERROR: Directory " +\
                                  dirtxt + " does not exist";
            perror ("");
            std::cout << outstr << std::endl;
            //return EXIT_FAILURE;
        }
    }
    in_file.close ();
}

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETNUMSTATIONS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int getNumStations ()
{
    const std::string dir = "./data/";
    int tempi, ipos, count = 0, nstations = 0;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt;

    fname = dir + "station_latlons_london.txt";
    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        std::cout << "***ERROR: Failed to read `station_latlons_london.txt'***" << 
            std::endl;
    } else {
        getline (in_file, linetxt, '\n');
        count = 0;
        while (getline (in_file, linetxt, '\n')) { count++;	}
        in_file.clear ();
        in_file.seekg (0); // Both lines needed to rewind file.
        for (int i=0; i<=count; i++) {
            getline (in_file, linetxt,'\n');
            ipos = linetxt.find(',',0);
            tempi = atoi (linetxt.substr (0, ipos).c_str());
            if (tempi > nstations) nstations = tempi;
        }
        in_file.close();
    }

    return nstations;
} // end function getNumStations


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           READLATLONS                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readLatLons (dvec* lons, dvec* lats)
{
    // Presumes file existence confirmed with getNumStations
    const std::string dir = "./data/";
    int count, ipos, tempi;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt;

    for (int i=0; i<(*lons).size (); i++) {
        (*lons) (i) = NAN;
        (*lats) (i) = NAN;	}

    fname = dir + "station_latlons_london.txt";
    in_file.open (fname.c_str (), std::ifstream::in);
    getline (in_file, linetxt, '\n');
    count = 0;
    while (getline (in_file, linetxt, '\n')) { count++;	}
    if (count > (*lons).size ()) {
        // INSERT ERROR HANDLER
    }
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');
    for (int i=0; i<count; i++) {
        getline (in_file, linetxt,'\n');
        ipos = linetxt.find(',',0);
        tempi = atoi (linetxt.substr (0, ipos).c_str());
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (',', 0);
        (*lats) (tempi - 1) = atof (linetxt.substr (0, ipos).c_str());
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (',', 0);
        (*lons) (tempi - 1) = atof (linetxt.substr (0, ipos).c_str());
    }
    in_file.close();
} // end function read_latlons


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETSTATIONNAMES                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void getStationNames (std::vector <std::string>* names)
{
    // Presumes file existence confirmed with getNumStations
    const std::string dir = "./data/";
    int count, ipos, tempi;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt;

    for (int i=0; i<(*names).size (); i++) (*names) [i].clear ();

    fname = dir + "station_latlons_london.txt";
    in_file.open (fname.c_str (), std::ifstream::in);
    getline (in_file, linetxt, '\n');
    count = 0;
    while (getline (in_file, linetxt, '\n')) { count++;	}
    if (count > (*names).size ()) {
        // INSERT ERROR HANDLER
    }
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');
    for (int i=0; i<count; i++) {
        getline (in_file, linetxt,'\n');
        ipos = linetxt.find(',',0);
        tempi = atoi (linetxt.substr (0, ipos).c_str());
        for (int j=0; j<2; j++) {
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find(',',0);
        }
        (*names) [tempi - 1] = 
            linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
    }
    in_file.close();
} // end function read_latlons


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             READDATA                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int readData (imat* ntrips, std::string fname)
{
    /*
     * Reads the tab-delimited .txt format data provided by the London hire bike
     * scheme. These have 9 columns of:
     * (1.Rental Id, 2.Duration, 3.Bike Id, 4.End Date, 5.EndStation Id, 
     * 6.EndStation Name, 7.Start Date, 8.StartStation Id, 9.StartStation Name)
     *
     * This routine presumes checkDataFile (), so there is no fail check for .open.
     */
    int count, ipos, tempi [2], nstations = (*ntrips).size1 ();
    std::ifstream in_file;
    std::string linetxt;

    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // INSERT ERROR HANDLER
        return 999;
    } 
    std::cout << "Reading file: " << fname.c_str ();
    std::cout.flush ();

    getline (in_file, linetxt, '\n');
    count = 0;
    while (getline (in_file, linetxt, '\n')) { count++;	}
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');

    for (int k=0; k<=count; k++) {
        getline (in_file, linetxt,'\n');
        for (int j=0; j<4; j++) {
            ipos = linetxt.find(',',0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (',', 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        for (int j=0; j<2; j++) {
            ipos = linetxt.find ('"', 0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        for (int j=0; j<2; j++) {
            ipos = linetxt.find(',',0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find(',',0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        if (tempi [0] > 0 && tempi [0] <= nstations && tempi [1] > 0 && 
                tempi [1] <= nstations) {
            (*ntrips) (tempi [1] - 1, tempi [0] - 1)++;
        } // end if 
    } // end for k
    in_file.close();
    std::cout << " done." << std::endl;

    return 0;
} // end function readData


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           TRIPNUMRANGE                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

ivec tripNumRange (imat* ntrips)
{
    int tempi, n = (*ntrips).size1 ();
    ivec counts;
    counts.resize (3); // total, max, and number of zero counts
    counts (0) = counts (1) = 0;

    for (int i=0; i<n; i++) {
        tempi = 0;
        for (int j=0; j<n; j++) {
            counts (0) += (*ntrips) (i, j);
            if ((*ntrips) (i, j) > counts (1)) counts (1) = (*ntrips) (i, j);
            if ((*ntrips) (i, j) > tempi) tempi = (*ntrips) (i, j);
        } // end for j
        if (tempi == 0) counts (2)++;
    } // end for i

    return counts;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            WRITEDATA                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void writeR2mat (dmat *r2mat, bool from)
{
    // NOTE that this presumes the directory ./results exists, and will crash if
    // not. Directory checks are system dependent, so this way at least remains
    // system independent.
    const std::string resultsDir = "./results/";
    std::string fname;
    std::ofstream out_file;
    if (from) fname = "r2from.csv";
    else fname = "r2to.csv";
    fname = resultsDir + fname;

    int nstations = (*r2mat).size1 ();

    out_file.open (fname.c_str (), std::ofstream::out);
    for (int i=0; i<nstations; i++) {
        for (int j=0; j<nstations; j++) {
            out_file << (*r2mat) (i, j);
            if (j < (nstations - 1)) { out_file << ",\t";	}
            else { out_file << std::endl;	}
        }
    }
    out_file.close ();
} // end writeVectorData

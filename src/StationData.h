/***************************************************************************
 * This software is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * <StationData.h>
 *
 * Author: Mark Padgham, May 2015
 ***************************************************************************/

#ifndef STATIONDATA_H
#define STATIONDATA_H

#include "Utils.h"
#include "Structures.h"

#include <dirent.h>
#include <stdlib.h> // for EXIT_FAILURE
#include <string.h>
#include <fstream>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!

class Stations // for both bikes and trains
{
    /*
     * Stations and BikeStation data are generic classes into which all data
     * is initially loaded, and the only classes on which subsequent
     * manipulations should be make. The descendent class RideData exists only
     * to load data into the standard StationData and BikeStation classes.
     */
    protected:
        std::string _dirName;
        const std::string _city;
        bool _standardise;
        // Standardises ntrips to unit sum, so covariances do not depend on
        // scales of actual numbers of trips. Set to true in initialisation.
    public:
        int nearfar;
        // nearfar determines whether correlations are calculated from all data,
        // only from the nearest 50% of stations, or only from the farthest 50%.
        // nearfar = 0 uses all data
        // nearfar = 1 uses only data from the nearest 50% of stations
        // nearfar = 2 uses only data from the farthest 50% of stations
        // Distances are calculated as sums of distances from both stations.
        std::string fileName;
        Stations (std::string str)
            : _city (str)
        {
            _standardise = true; // false doesn't make sense
            _dirName = GetDirName ();
        }
        ~Stations ()
        {
        }

        int getStandardise () { return _standardise;    }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }
        
        std::string GetDirName ();
};


class StationData : public Stations
{
    protected:
        int _numStations, _maxStation;
        std::vector <int> _StationIndex;
        std::string _nearfarTxt [3];
    public:
        int tempi;
        std::vector <int> missingStations;
        struct OneStation 
        {
            std::string name; // for train stations only
            int ID;
            float lon, lat;
        };
        std::vector <OneStation> StationList;
        dmat ntrips; // dmat to allow standardisation to unit sum
        dmat r2, cov, MI, dists;

        std::string txtnf;
        std::vector <std::string> txtnflist;

        StationData (std::string str)
            : Stations (str)
        {
            GetDirList ();
            _maxStation = GetStations ();
            _numStations = StationList.size ();
            missingStations.resize (0);
            InitialiseArrays ();
            if (_city.substr (0, 6) != "oyster")
                MakeStationIndex ();

            txtnflist.resize (0);
            txtnflist.push_back ("all");
            txtnflist.push_back ("near");
            txtnflist.push_back ("far");
        }
        ~StationData()
        {
            filelist.resize (0);
            missingStations.resize (0);
            ntrips.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            MI.resize (0, 0);
            dists.resize (0, 0);
            txtnflist.resize (0);
        }

        int returnNumStations () { return _numStations; }
        int returnMaxStation () { return _maxStation;   }

        std::vector <std::string> filelist;
        void GetDirList ();
        int GetStations ();
        void MakeStationIndex ();
        double CountTrips ();

        int readDMat ();
        int writeDMat ();
        int writeNumTrips (std::string fname);

        int calcR2 (bool from);
        int writeR2Mat (std::string fname);
        int writeCovMat (std::string fname);
        double calcMI (dvec x, dvec y);
        int calcMIMat (bool from);
        int writeMIMat (std::string fname);

        void InitialiseArrays ()
        {
            ntrips.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            MI.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips (i, j) = 0.0;
                    r2 (i, j) = DOUBLE_MIN;
                    cov (i, j) = DOUBLE_MIN;
                    MI (i, j) = DOUBLE_MIN;
                    dists (i, j) = DOUBLE_MIN;
                }
            }
        }
}; // end class StationData

#endif

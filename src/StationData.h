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
        dmat dists2;
    public:
        std::string fileName;
        Stations (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
        }
        ~Stations ()
        {
        }

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
        std::vector <int> missingStations;
        struct OneStation 
        {
            std::string name; // for train stations only
            int ID;
            float lon, lat;
        };
        std::vector <OneStation> StationList;
        dmat ntrips; // dmat to allow standardisation to unit sum
        dmat r2, cov, dists;

        StationData (std::string str)
            : Stations (str)
        {
            GetDirList ();
            _maxStation = GetStations ();
            _numStations = StationList.size ();
            missingStations.resize (0);
            MakeStationIndex ();
            InitialiseArrays ();
        }
        ~StationData()
        {
            filelist.resize (0);
            missingStations.resize (0);
            ntrips.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
        }

        int returnNumStations () { return _numStations; }
        int returnMaxStation () { return _maxStation;   }

        std::vector <std::string> filelist;
        void GetDirList ();
        int GetStations ();
        void MakeStationIndex ();

        void InitialiseArrays ()
        {
            ntrips.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    //ntrips (i, j) = 0.0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
        }
}; // end class StationData


#endif

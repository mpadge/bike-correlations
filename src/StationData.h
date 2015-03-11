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

class StationData 
{
    private:
        std::string _dirName;
        const std::string _city;
        bool _standardise;
        bool _ignoreZeros;
        const int _nearfar, _subscriber, _gender;
        // nearfar determines whether correlations are calculated from all data,
        // only from the nearest 50% of stations, or only from the farthest 50%.
        // nearfar = 0 uses all data
        // nearfar = 1 uses only data from the nearest 50% of stations
        // nearfar = 2 uses only data from the farthest 50% of stations
        // Distances are calculated as sums of distances from both stations.
        // subscriber = (0, 1, 2) for (all, subscriber, customer)
        // gender = (0, 1, 2) for (all, male, female)
        std::string _nearfarTxt [3];
    protected:
        int _numStations, _maxStations;
        std::vector <std::string> FileList;
        std::vector <int> _StationIndex;
    public:
        StationData (std::string str, bool b0, int i0, int i1, int i2)
            : _city (str), _ignoreZeros (b0), _nearfar (i0), 
                _subscriber (i1), _gender (i2)
        {
            _dirName = GetDirName ();
            GetDirList ();
            FileList = filelist;
            _numStations = GetStations ();
            MakeStationIndex ();
        }
        ~StationData ()
        {
            filelist.resize (0);
            FileList.resize (0);
        }
        struct OneStation {
            int ID, indx;
            float lon, lat;
        };
        std::vector <OneStation> StationList;
        std::vector <std::string> filelist;

        std::string GetDirName ();
        void GetDirList ();
        void MakeStationIndex ();
        int GetStations ();

        int getNumStations () { return _numStations;  }
        int getMaxStation () { return _maxStations;  }
        int getNearfar () { return _nearfar;    }
        int getSubscriber () { return _subscriber;  }
        int getGender () { return _gender;  }
        bool getIgnoreZeros () { return _ignoreZeros;    }
        std::string getCity () { return _city;    }
        std::vector <int> getStationIndex() { return _StationIndex; }
};


#endif

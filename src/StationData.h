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
    protected:
        int _numStations, _maxStations;
        std::vector <std::string> FileList;
        std::vector <int> _StationIndex;
    public:
        StationData (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
            GetDirList ();
            FileList = filelist;
            _numStations = GetStations ();
            MakeStationIndex ();
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
        std::string getCity () { return _city;    }
        std::vector <int> getStationIndex() { return _StationIndex; }
};


#endif

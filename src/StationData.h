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

class StationData // for both bikes and trains
{
    protected:
        int _numStations;
        std::string _dirName;
        const std::string _city;
        bool _standardise;
    public:
        std::string fileName;
        StationData (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
        }
        ~StationData ()
        {
        }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }
        int returnNumStations () { return _numStations; }
        
        std::string GetDirName ();
};


class BikeStationData : public StationData
{
    protected:
        int _maxStation;
        std::vector <int> _StationIndex;
        std::string _nearfarTxt [3];
    public:
        std::vector <int> missingStations;
        struct OneStation 
        {
            int ID;
            float lon, lat;
        };
        std::vector <OneStation> StationList;

        BikeStationData (std::string str)
            : StationData (str)
        {
            GetDirList ();
            GetStations ();
            _numStations = StationList.size ();
            missingStations.resize (0);
            MakeStationIndex ();
        }
        ~BikeStationData()
        {
            filelist.resize (0);
            missingStations.resize (0);
        }

        int returnMaxStation () { return _maxStation;   }

        std::vector <std::string> filelist;
        void GetDirList ();
        void GetStations ();
        void MakeStationIndex ();
}; // end class BikeStationData

class TrainStationData 
{
    private:
        std::string _dirName, _city = "oyster";
        bool _standardise;
        std::string _nearfarTxt [3];
    protected:
        int _numRailStations, _numTubeStations;
        struct oysterOne
        {
            std::string mode, name; // mode is NR, LUL, DLR
        };
        std::vector <int> _StationIndex;
        std::vector <oysterOne> _OysterStations;
    public:
        TrainStationData ()
        {
            _dirName = GetDirName ();
            _numRailStations = GetRailStations ();
            _numTubeStations = GetTubeStations ();
        }
        ~TrainStationData ()
        {
            _StationIndex.resize (0);
        }
        struct OneStation
        {
            int ID, indx;
            float lon, lat;
        };
        std::vector <std::string> RailStationList;
        std::vector <std::string> TubeStationList;

        std::string GetDirName ();
        void GetDirList ();
        void MakeStationIndex ();
        int GetStations ();
        int GetRailStations ();
        int GetTubeStations ();

        int getNumRailStations () { return _numRailStations;  }
        int getNumTubeStations () { return _numTubeStations;  }
        std::vector <int> getStationIndex() { return _StationIndex; }
}; // end class TrainStationData


#endif

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

class BikeStationData 
{
    private:
        std::string _dirName;
        const std::string _city;
        bool _standardise;
        std::string _nearfarTxt [3];
    protected:
        int _numStations, _maxStations;
        std::vector <std::string> FileList;
        std::vector <int> _StationIndex;
    public:
        BikeStationData (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
            GetDirList ();
            FileList = filelist;
            _numStations = GetStations ();
            MakeStationIndex ();
        }
        ~BikeStationData ()
        {
            filelist.resize (0);
            FileList.resize (0);
            _StationIndex.resize (0);
        }
        struct OneStation
        {
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
}; // end class BikeStationData


class TrainStationData 
{
    private:
        std::string _dirName;
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

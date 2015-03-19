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
        int _numStations, _maxStations, _numRailStations, _numTubeStations;
        struct oysterOne
        {
            std::string mode, name; // mode is NR, LUL, DLR
        };
        std::vector <std::string> FileList;
        std::vector <int> _StationIndex;
        std::vector <oysterOne> _OysterStations;
    public:
        BikeStationData (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
            if (_city == "oyster")
            {
                _numRailStations = GetRailStations ();
                _numTubeStations = GetTubeStations ();
            }
            else
            {
                GetDirList ();
                FileList = filelist;
                _numStations = GetStations ();
                MakeStationIndex ();
            }
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
        std::vector <std::string> RailStationList;
        std::vector <std::string> TubeStationList;
        std::vector <std::string> filelist;

        std::string GetDirName ();
        void GetDirList ();
        void MakeStationIndex ();
        int GetStations ();
        int GetRailStations ();
        int GetTubeStations ();

        int getNumStations () { return _numStations;  }
        int getMaxStation () { return _maxStations;  }
        std::string getCity () { return _city;    }
        std::vector <int> getStationIndex() { return _StationIndex; }
}; // end class BikeStationData


class TrainStationData 
{
    private:
        std::string _dirName;
        const std::string _city;
        bool _standardise;
        std::string _nearfarTxt [3];
    protected:
        int _numStations, _maxStations, _numRailStations, _numTubeStations;
        struct oysterOne
        {
            std::string mode, name; // mode is NR, LUL, DLR
        };
        std::vector <std::string> FileList;
        std::vector <int> _StationIndex;
        std::vector <oysterOne> _OysterStations;
    public:
        TrainStationData (std::string str)
            : _city (str)
        {
            _dirName = GetDirName ();
            if (_city == "oyster")
            {
                _numRailStations = GetRailStations ();
                _numTubeStations = GetTubeStations ();
            }
            else
            {
                GetDirList ();
                FileList = filelist;
                _numStations = GetStations ();
                MakeStationIndex ();
            }
        }
        ~TrainStationData ()
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
        std::vector <std::string> RailStationList;
        std::vector <std::string> TubeStationList;
        std::vector <std::string> filelist;

        std::string GetDirName ();
        void GetDirList ();
        void MakeStationIndex ();
        int GetStations ();
        int GetRailStations ();
        int GetTubeStations ();

        int getNumStations () { return _numStations;  }
        int getMaxStation () { return _maxStations;  }
        std::string getCity () { return _city;    }
        std::vector <int> getStationIndex() { return _StationIndex; }
}; // end class TrainStationData


#endif

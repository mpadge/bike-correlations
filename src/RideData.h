#ifndef RIDEDATA_H
#define RIDEDATA_H

#include "Utils.h"
#include "Structures.h"
#include "StationData.h"

#include <zip.h>
#include <errno.h>

class RideData: public StationData 
{
    /*
     * Exists only to read and load data into the base StationData class.
     */
    private:
        int _numTripFiles, _stnIndxLen;
        const int _subscriber, _gender;
        // subscriber = (0, 1, 2) for (all, subscriber, customer)
        // gender = (0, 1, 2) for (all, male, female)
    public:
        imat ntrips_cust, ntrips_sub_m, ntrips_sub_f, ntrips_sub_n;
        imat ntrips1920, ntrips1930, ntrips1940, ntrips1950, ntrips1960,
             ntrips1970, ntrips1980, ntrips1990, ntrips2000,
             ntripsYoung, ntripsOld;
        int ageDistribution [99];
        // Customers by definition have no data, and the _n files are
        // subscribers whose gender is not given
        std::string fileName;
        std::vector <int> missingStations;

        RideData (std::string str, int i0, int i1)
            : StationData (str), _subscriber (i0), _gender (i1)
        {
            _numTripFiles = filelist.size ();
            _stnIndxLen = _StationIndex.size ();
            missingStations.resize (0);
            if (_city == "nyc" || _city == "boston")
            {
                if (_subscriber < 3)
                    subscriberMFConstruct ();
                else
                    subscriberAgeConstruct ();
                for (int i=0; i<99; i++)
                    ageDistribution [i] = 0;
            }
        }

        ~RideData ()
        {
            missingStations.resize (0);
            subscriberMFDestruct();
            subscriberAgeDestruct();
        }
        

        int getSubscriber () { return _subscriber;  }
        int getGender () { return _gender;  }

        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }

        int countFilesLondon (int file);
        int unzipOneFileLondon (int filei, int filej);
        int readOneFileLondon ();
        void dumpMissingStations ();
        int removeFile ();

        int getZipFileNameNYC (int filei);
        int readOneFileNYC (int filei);
        void summaryStats ();

        int getZipFileNameBoston (int filei);
        int readOneFileBoston (int filei);
        
        int aggregateTrips ();

        int readR2Mat (bool from);

        void subscriberMFConstruct()
        {
            ntrips_cust.resize (_numStations, _numStations);
            ntrips_sub_f.resize (_numStations, _numStations);
            ntrips_sub_m.resize (_numStations, _numStations);
            ntrips_sub_n.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips_cust (i, j) = 0;
                    ntrips_sub_f (i, j) = 0;
                    ntrips_sub_m (i, j) = 0;
                    ntrips_sub_n (i, j) = 0;
                }
            }
        }
        void subscriberAgeConstruct()
        {
            ntrips1920.resize (_numStations, _numStations);
            ntrips1930.resize (_numStations, _numStations);
            ntrips1940.resize (_numStations, _numStations);
            ntrips1950.resize (_numStations, _numStations);
            ntrips1960.resize (_numStations, _numStations);
            ntrips1970.resize (_numStations, _numStations);
            ntrips1980.resize (_numStations, _numStations);
            ntrips1990.resize (_numStations, _numStations);
            ntrips2000.resize (_numStations, _numStations);
            ntripsYoung.resize (_numStations, _numStations);
            ntripsOld.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips1920 (i, j) = 0;
                    ntrips1930 (i, j) = 0;
                    ntrips1940 (i, j) = 0;
                    ntrips1950 (i, j) = 0;
                    ntrips1960 (i, j) = 0;
                    ntrips1970 (i, j) = 0;
                    ntrips1980 (i, j) = 0;
                    ntrips1990 (i, j) = 0;
                    ntrips2000 (i, j) = 0;
                    ntripsYoung (i, j) = 0;
                    ntripsOld (i, j) = 0;
                }
            }
        }
        void subscriberMFDestruct()
        {
            ntrips_cust.resize (0, 0);
            ntrips_sub_f.resize (0, 0);
            ntrips_sub_m.resize (0, 0);
            ntrips_sub_n.resize (0, 0);
        }
        void subscriberAgeDestruct ()
        {
            ntrips1920.resize (0, 0);
            ntrips1930.resize (0, 0);
            ntrips1940.resize (0, 0);
            ntrips1950.resize (0, 0);
            ntrips1960.resize (0, 0);
            ntrips1970.resize (0, 0);
            ntrips1980.resize (0, 0);
            ntrips1990.resize (0, 0);
            ntrips2000.resize (0, 0);
            ntripsYoung.resize (0, 0);
            ntripsOld.resize (0, 0);
        }
};

#endif

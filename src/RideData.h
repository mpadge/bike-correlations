#ifndef RIDEDATA_H
#define RIDEDATA_H

#include "Utils.h"
#include "Structures.h"
#include "StationData.h"

#include <zip.h>
#include <errno.h>

class RideData: public StationData 
{
    private:
        int _numTripFiles, _stnIndxLen;
        bool _standardise;
        // Standardises ntrips to unit sum, so covariances do not depend on
        // scales of actual numbers of trips. Set to true in initialisation.
        const int _subscriber, _gender;
        // nearfar determines whether correlations are calculated from all data,
        // only from the nearest 50% of stations, or only from the farthest 50%.
        // nearfar = 0 uses all data
        // nearfar = 1 uses only data from the nearest 50% of stations
        // nearfar = 2 uses only data from the farthest 50% of stations
        // Distances are calculated as sums of distances from both stations.
        // subscriber = (0, 1, 2) for (all, subscriber, customer)
        // gender = (0, 1, 2) for (all, male, female)
    public:
        bool ignoreZeros;
        int nearfar;
        dmat ntrips; // dmat to allow standardisation to unit sum
        imat ntrips_cust, ntrips_sub_m, ntrips_sub_f, ntrips_sub_n;
        // Customers by definition have no data, and the _n files are
        // subscribers whose gender is not given
        dmat r2, cov, dists;
        std::string fileName;
        std::vector <int> missingStations;
        std::string txtzero, txtnf;
        std::vector <std::string> txtzerolist, txtnflist;

        RideData (std::string str, int i0, int i1)
            : StationData (str), _subscriber (i0), _gender (i1)
        {
            _numStations = getNumStations();
            _numTripFiles = filelist.size ();
            _stnIndxLen = _StationIndex.size ();
            missingStations.resize (0);
            ntrips.resize (_numStations, _numStations);
            ntrips_cust.resize (_numStations, _numStations);
            ntrips_sub_f.resize (_numStations, _numStations);
            ntrips_sub_m.resize (_numStations, _numStations);
            ntrips_sub_n.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips_cust (i, j) = 0;
                    ntrips_sub_f (i, j) = 0;
                    ntrips_sub_m (i, j) = 0;
                    ntrips_sub_n (i, j) = 0;
                    ntrips (i, j) = 0.0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
            _standardise = true; // false doesn't make sense

            txtzerolist.resize (0);
            txtzerolist.push_back ("zeros");
            txtzerolist.push_back ("nozeros");
            txtnflist.resize (0);
            txtnflist.push_back ("all");
            txtnflist.push_back ("near");
            txtnflist.push_back ("far");
        }

        ~RideData ()
        {
            missingStations.resize (0);
            ntrips.resize (0, 0);
            ntrips_cust.resize (0, 0);
            ntrips_sub_f.resize (0, 0);
            ntrips_sub_m.resize (0, 0);
            ntrips_sub_n.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
            txtzerolist.resize (0);
            txtnflist.resize (0);
        }
        
        int getSubscriber () { return _subscriber;  }
        int getGender () { return _gender;  }

        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }
        int getStandardise () { return _standardise;    }

        int countFilesLondon (int file);
        int unzipOneFileLondon (int filei, int filej);
        int readOneFileLondon ();
        void dumpMissingStations ();
        int removeFile ();

        int getZipFileNameNYC (int filei);
        int readOneFileNYC (int filei);
        int aggregateTrips ();

        int writeNumTrips ();
        int calcR2 (bool from);
        int writeR2Mat (bool from);
        int writeCovMat (bool from);
        int readR2Mat (bool from);
        int writeDMat ();
};

#endif

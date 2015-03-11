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
    public:
        dmat ntrips; // dmat to allow standardisation to unit sum
        imat ntrips_cust_m, ntrips_cust_f, ntrips_sub_m, ntrips_sub_f;
        dmat r2, cov, dists;
        std::string fileName, nfext;
        std::vector <int> missingStations;

        RideData (std::string str, bool b0, int i0, int i1, int i2)
            : StationData (str, b0, i0, i1, i2)
        {
            _numStations = getNumStations();
            _numTripFiles = filelist.size ();
            _stnIndxLen = _StationIndex.size ();
            missingStations.resize (0);
            ntrips.resize (_numStations, _numStations);
            ntrips_cust_f.resize (_numStations, _numStations);
            ntrips_cust_m.resize (_numStations, _numStations);
            ntrips_sub_f.resize (_numStations, _numStations);
            ntrips_sub_m.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips_cust_f (i, j) = ntrips_cust_m (i, j) = 0;
                    ntrips_sub_f (i, j) = ntrips_sub_m (i, j) = 0;
                    ntrips (i, j) = 0.0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
            _standardise = true; // false doesn't make sense
        }

        ~RideData ()
        {
            missingStations.resize (0);
            ntrips.resize (0, 0);
            ntrips_cust_f.resize (0, 0);
            ntrips_cust_m.resize (0, 0);
            ntrips_sub_f.resize (0, 0);
            ntrips_sub_m.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
        }
        
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

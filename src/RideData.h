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
    public:
        imat ntrips;
        dmat r2, cov, dists;
        std::string fileName;
        std::vector <int> missingStations;

        RideData (std::string str)
            : StationData (str)
        {
            _numStations = getNumStations();
            _numTripFiles = filelist.size ();
            _stnIndxLen = _StationIndex.size ();
            missingStations.resize (0);
            ntrips.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips (i, j) = 0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
        }

        ~RideData ()
        {
            missingStations.resize (0);
            ntrips.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
        }
        
        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }

        int countFilesLondon (int file);
        int unzipOneFileLondon (int filei, int filej);
        int readOneFileLondon ();
        void dumpMissingStations ();
        int removeFile ();

        int getZipFileNameNYC (int filei);
        int readOneFileNYC (int filei);

        int writeNumTrips ();
        int calcR2 (bool from);
        int writeR2Mat (bool from);
        int writeCovMat (bool from);
        int readR2Mat (bool from);
        int writeDMat ();
};

#endif

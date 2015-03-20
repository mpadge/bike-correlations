#ifndef RIDEDATA_H
#define RIDEDATA_H

#include "Utils.h"
#include "Structures.h"
#include "StationData.h"

#include <zip.h>
#include <errno.h>

class TrainData: public TrainStationData 
{
    private:
        int _numTripFiles, _stnIndxLen;
        bool _standardise;
        // Standardises ntrips to unit sum, so covariances do not depend on
        // scales of actual numbers of trips. Set to true in initialisation.
    public:
        bool ignoreZeros;
        int nearfar;
        dmat ntrips; // dmat to allow standardisation to unit sum
        imat ntrips_rail, ntrips_tube;
        dmat r2, cov, dists;
        std::string fileName;
        std::vector <int> missingStations;
        std::string txtzero, txtnf;
        std::vector <std::string> txtzerolist, txtnflist;

        TrainData ()
            : TrainStationData ()
        {
            _numRailStations = getNumRailStations ();
            _numTubeStations = getNumTubeStations ();
            ntrips_rail.resize (_numRailStations, _numRailStations);
            ntrips_tube.resize (_numTubeStations, _numTubeStations);
            subscriberOysterConstruct ();
            _standardise = true; // false doesn't make sense

            txtzerolist.resize (0);
            txtzerolist.push_back ("zeros");
            txtzerolist.push_back ("nozeros");
            txtnflist.resize (0);
            txtnflist.push_back ("all");
            txtnflist.push_back ("near");
            txtnflist.push_back ("far");
        }

        ~TrainData ()
        {
            missingStations.resize (0);
            ntrips.resize (0, 0);
            subscriberRailDestruct();
            r2.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
            txtzerolist.resize (0);
            txtnflist.resize (0);
        }
        

        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }
        int getStandardise () { return _standardise;    }
        int getNumRailStations () { return _numRailStations;    }
        int getNumTubeStations () { return _numTubeStations;   }

        int removeFile ();

        int getTrainStations ();
        int getTrainTrips ();

        int writeNumTrips ();
        int calcR2 (bool from);
        int writeR2Mat (bool from);
        int writeCovMat (bool from);
        int readR2Mat (bool from);
        int writeDMat ();

        void subscriberOysterConstruct()
        {
            ntrips_rail.resize (_numRailStations, _numRailStations);
            for (int i=0; i<_numRailStations; i++)
                for (int j=0; j<_numRailStations; j++)
                    ntrips_rail (i, j) = 0;
            ntrips_tube.resize (_numTubeStations, _numTubeStations);
            for (int i=0; i<_numTubeStations; i++)
                for (int j=0; j<_numTubeStations; j++)
                    ntrips_tube (i, j) = 0;
        }
        void InitialiseArrays ()
        {
            r2.resize (_numRailStations, _numRailStations);
            cov.resize (_numRailStations, _numRailStations);
            dists.resize (_numRailStations, _numRailStations);
            for (int i=0; i<_numRailStations; i++)
            {
                for (int j=0; j<_numRailStations; j++)
                {
                    ntrips (i, j) = 0.0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
        }
        void subscriberRailDestruct()
        {
            ntrips_rail.resize (0, 0);
            ntrips_tube.resize (0, 0);
        }
};

#endif

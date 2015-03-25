#ifndef TRAINDATA_H
#define TRAINDATA_H

#include "Utils.h"
#include "Structures.h"
#include "StationData.h"

#include <zip.h>
#include <errno.h>

class TrainData: public StationData 
{
    private:
        int _numTripFiles, _stnIndxLen;
        bool _standardise;
        const bool _tube;
        // Standardises ntrips to unit sum, so covariances do not depend on
        // scales of actual numbers of trips. Set to true in initialisation.
    protected:
    public:
        bool ignoreZeros;
        int err, nearfar;
        dmat ntrips; // dmat to allow standardisation to unit sum
        imat ntripsRail, ntripsTube;
        dmat r2Rail, r2Tube, covRail, covTube, distsRail, distsTube;
        std::string fileName;
        std::vector <int> missingStations;
        std::string txtnf;
        std::vector <std::string> txtnflist;

        TrainData (std::string str, bool tube)
            : StationData (str), _tube (tube)
        {
            err = getTrainData (_tube);
            _standardise = true; // false doesn't make sense
            InitialiseArrays ();

            txtnflist.resize (0);
            txtnflist.push_back ("all");
            txtnflist.push_back ("near");
            txtnflist.push_back ("far");
        }

        ~TrainData ()
        {
            missingStations.resize (0);
            ntrips.resize (0, 0);
            r2.resize (0, 0);
            r2.resize (0, 0);
            cov.resize (0, 0);
            cov.resize (0, 0);
            dists.resize (0, 0);
            txtnflist.resize (0);
        }
        

        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }
        int getStandardise () { return _standardise;    }

        int getNumStations (bool tube);
        int getTrainStations ();
        int getTrainData (bool tube);


        void InitialiseArrays ()
        {
            ntrips.resize (_numStations, _numStations);
            r2.resize (_numStations, _numStations);
            cov.resize (_numStations, _numStations);
            dists.resize (_numStations, _numStations);
            for (int i=0; i<_numStations; i++)
            {
                for (int j=0; j<_numStations; j++)
                {
                    ntrips (i, j) = 0.0;
                    r2 (i, j) = -9999.9;
                    cov (i, j) = -9999.9;
                    dists (i, j) = -9999.9;
                }
            }
        }
};

#endif

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
        imat ntripsRail, ntripsTube;
        dmat r2Rail, r2Tube, covRail, covTube, distsRail, distsTube;
        std::string fileName;
        std::vector <int> missingStations;
        std::string txtzero, txtnf;
        std::vector <std::string> txtzerolist, txtnflist;

        TrainData ()
            : TrainStationData ()
        {
            _numRailStations = getNumRailStations ();
            _numTubeStations = getNumTubeStations ();
            ntripsRail.resize (_numRailStations, _numRailStations);
            ntripsTube.resize (_numTubeStations, _numTubeStations);
            TrainDataConstruct ();
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
            r2Rail.resize (0, 0);
            r2Tube.resize (0, 0);
            covRail.resize (0, 0);
            covTube.resize (0, 0);
            distsRail.resize (0, 0);
            distsTube.resize (0, 0);
            ntripsRail.resize (0, 0);
            ntripsTube.resize (0, 0);
            txtzerolist.resize (0);
            txtnflist.resize (0);
        }
        

        int getNumFiles () { return _numTripFiles;  }
        int getStnIndxLen () { return _stnIndxLen;  }
        int getStandardise () { return _standardise;    }

        int getTrainStations ();
        int getTrainTrips ();

        int writeNumTrips ();
        int calcR2 (bool from);
        int writeR2Mat (bool from);
        int writeCovMat (bool from);
        int readR2Mat (bool from);
        int writeDMat ();

        void TrainDataConstruct()
        {
            ntripsRail.resize (_numRailStations, _numRailStations);
            for (int i=0; i<_numRailStations; i++)
                for (int j=0; j<_numRailStations; j++)
                    ntripsRail (i, j) = 0;
            ntripsTube.resize (_numTubeStations, _numTubeStations);
            for (int i=0; i<_numTubeStations; i++)
                for (int j=0; j<_numTubeStations; j++)
                    ntripsTube (i, j) = 0;
        }
        void InitialiseArrays ()
        {
            r2Rail.resize (_numRailStations, _numRailStations);
            covRail.resize (_numRailStations, _numRailStations);
            distsRail.resize (_numRailStations, _numRailStations);
            for (int i=0; i<_numRailStations; i++)
            {
                for (int j=0; j<_numRailStations; j++)
                {
                    ntripsRail (i, j) = 0.0;
                    r2Rail (i, j) = -9999.9;
                    covRail (i, j) = -9999.9;
                    distsRail (i, j) = -9999.9;
                }
            }
            r2Tube.resize (_numRailStations, _numRailStations);
            covTube.resize (_numRailStations, _numRailStations);
            distsTube.resize (_numRailStations, _numRailStations);
            for (int i=0; i<_numTubeStations; i++)
            {
                for (int j=0; j<_numTubeStations; j++)
                {
                    ntripsTube (i, j) = 0.0;
                    r2Tube (i, j) = -9999.9;
                    covTube (i, j) = -9999.9;
                    distsTube (i, j) = -9999.9;
                }
            }
        }
};

#endif

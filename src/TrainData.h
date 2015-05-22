/***************************************************************************
 * This software is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * <TrainData.h>
 *
 * Author: Mark Padgham, May 2015
 ***************************************************************************/

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
        const bool _tube;
    protected:
        int numStnsWithData;
        std::vector <bool> hasData;
    public:
        int count;
        std::string fname, txtnf;
        std::vector <std::string> txtnflist;

        struct OysterIndx
        {
            std::string name;
            int index;
        };
        std::vector <OysterIndx> Oyster2StnIndex;

        TrainData (std::string str, bool tube)
            : StationData (str), _tube (tube)
        {
            hasData.resize (0);
            Oyster2StnIndex.resize (0);
            count = readDMat ();
            count = readOysterData ();
            count = CountTrips ();
            numStnsWithData = fillHasData ();
            count = resizeNtrips ();
            count = writeDMat ();
            if (numStnsWithData != Oyster2StnIndex.size ())
                std::cout << "ERROR: hasData has " << hasData.size() << 
                    " stations and Oyster2StnIndex has " << 
                    Oyster2StnIndex.size () << std::endl;
            if (_city == "oysterTube")
                fname = "NumTrips_london_tube.csv";
            else
                fname = "NumTrips_london_rail.csv";
            count = writeNumTrips (fname);

            txtnflist.resize (0);
            txtnflist.push_back ("all");
            txtnflist.push_back ("near");
            txtnflist.push_back ("far");
        }

        ~TrainData ()
        {
            hasData.resize (0);
            txtnflist.resize (0);
        }
        
        int readOysterData ();
        int fillHasData ();
        int resizeNtrips ();
};

#endif

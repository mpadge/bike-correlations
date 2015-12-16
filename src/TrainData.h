/***************************************************************************
 *  Project:    bike-correlations
 *  File:       TrainData.h
 *  Language:   C++
 *
 *  bike-correlations is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  bike-correlations is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along with
 *  NeutralClusters.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright   Mark Padgham December 2015
 *  Author:     Mark Padgham
 *  E-Mail:     mark.padgham@email.com
 *
 *  Description:    Constructs correltaion matrices between all stations of
 *                  public bicycle hire systems for London, UK, and Boston,
 *                  Chicago, Washington DC, and New York, USA. Also analyses
 *                  Oystercard data for London.
 *
 *  Limitations:
 *
 *  Dependencies:       libboost
 *
 *  Compiler Options:   -std=c++11 -lboost_program_options -lzip
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

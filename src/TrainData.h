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
        std::vector <bool> hasData;
    public:
        int count, nearfar;
        std::string txtnf;
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
            count = readOysterData ();
            count = CountTrips ();
            count = fillHasData ();
            if (count != Oyster2StnIndex.size ())
                std::cout << "ERROR: hasData has " << count << 
                    " stations and Oyster2StnIndex has " << 
                    Oyster2StnIndex.size () << std::endl;


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
};

#endif

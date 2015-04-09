#include "Utils.h"

#include <tuple>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/connected_components.hpp>

typedef std::pair <double, double> ddPair;

typedef boost::unordered_map <long long, int> umapInt;
typedef boost::unordered_map <long long, int>::iterator umapInt_Itr;
typedef boost::unordered_map <long long, ddPair> umapPair;
typedef boost::unordered_map <long long, ddPair>::iterator umapPair_Itr;

struct Segment
{
    long long from, to;
    float d;
};
struct Station
{
    float lon, lat;
    long long node;
    int nodeIndex;
};

// See http://theboostcpplibraries.com/boost.unordered
std::size_t hash_value(const int &i)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, i);
    return seed;
}
std::size_t hash_value(const ddPair &d)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, d.first);
    boost::hash_combine(seed, d.second);
    return seed;
}


class Ways 
{
    protected:
        std::string _dirName;
        const std::string _city;
    public:
        int err;
        long long node;
        float d;
        /*
         * OSM data are read into allNodes, with most node IDs being much bigger
         * than MAX_INT. Boost graphs member function id() used to reference
         * vertices returns an int, and thus passing any values > MAX_INT causes
         * a bad allocation. The way around this here is to index nodes with
         * nodeNames, which just renumbers all terminal nodes to consecutive
         * ints, rather than huge long longs.
         *
         * The storage of nodeNames is done in readWays, while they are then
         * only subsequently used to replace the long long OSM numbers with
         * corresponding ints when storing the boost::graph in the sp routine.
         */
        umapPair allNodes;
        umapInt nodeNames;
        std::vector <Segment> wayList;
        std::vector <Station> stationList;
        std::vector <float> dists;
        Ways (std::string str)
            : _city (str)
        {
            err = readNodes();
            err = readWays ();
            err = readStations ();
            /*
            for (std::vector<Station>::iterator itr = stationList.begin();
                    itr != stationList.end(); itr++)
            {
                err = sp (itr->nodeIndex);
                std::cout << "Extracted " << err << " valid distances" << std::endl;
            }
            */
            err = sp (stationList.begin()->nodeIndex);
            std::cout << "Extracted " << err << " valid distances" << std::endl;
        }
        ~Ways ()
        {
            wayList.resize (0);
            stationList.resize (0);
            dists.resize (0);
        }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }

        int readNodes ();
        int readTerminalNodes ();
        int readWays ();
        int readStations ();
        int sp (long long fromNode);

        float calcDist (float x0, float y0, float x1, float y1);
        long long nearestNode (float lon0, float lat0);
};

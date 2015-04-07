#include "Utils.h"

#include <tuple>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

typedef std::pair <double, double> ddPair;

typedef boost::unordered_set <long long> uset;
typedef boost::unordered_set <long long>::iterator uset_Itr;
typedef boost::unordered_map <long long, int> umapInt;
typedef boost::unordered_map <long long, int>::iterator umapInt_Itr;
typedef boost::unordered_map <long long, ddPair> umapPair;
typedef boost::unordered_map <long long, ddPair>::iterator umapPair_Itr;

struct segment
{
    long long from, to;
    float d;
};

// See http://theboostcpplibraries.com/boost.unordered
/*
std::size_t hash_value(const ddPair &d)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, d.first);
    boost::hash_combine(seed, d.second);
    return seed;
}
*/


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
         * OSM data are read into terminalNodeIDs and allNodes, with most node
         * IDs being much bigger than MAX_INT. The routing is done with
         * boost::dijkstra, which is hard-coded to treat node IDs as ints, thus
         * passing any values > MAX_INT causes a bad allocation. The way around
         * this here is to create a second nodeNames, which just renumbers all
         * terminal nodes to consecutive ints, rather than huge long longs.
         *
         * The storage of nodeNames is done in readTerminalNodes, while they are
         * then only subsequently used to replace the long long OSM numbers with
         * corresponding ints when storing the boost::graph in the sp routine.
         */
        uset terminalNodeIDs;
        umapPair allNodes;
        umapInt nodeNames;
        std::vector <segment> wayList;
        Ways (std::string str)
            : _city (str)
        {
            err = readNodes();
            err = readTerminalNodes ();
            err = readWays ();
            d = sp (*terminalNodeIDs.begin());
        }
        ~Ways ()
        {
            wayList.resize (0);
        }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }

        int readNodes ();
        int readTerminalNodes ();
        int readWays ();
        float sp (long long fromNode);
        float spOld (long long fromNode);

        float calcDist (std::vector <float> x, std::vector <float> y);
};

#include "Utils.h"

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

typedef std::pair <double, double> ddPair;

typedef boost::unordered_set <long long> uset;
typedef boost::unordered_set <long long>::iterator uset_Itr;
typedef boost::unordered_map <long long, ddPair> umapPair;
typedef boost::unordered_map <long long, ddPair>::iterator umapPair_Itr;

class Ways 
{
    protected:
        std::string _dirName;
        const std::string _city;
    public:
        int err;
        uset terminalNodeIDs;
        umapPair allNodes;
        Ways (std::string str)
            : _city (str)
        {
            err = readNodes();
            err = readTerminalNodes ();
            err = readWays ();
        }
        ~Ways ()
        {
        }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }

        int readNodes ();
        int readTerminalNodes ();
        int readWays ();

        double calcDist (std::vector <double> x, std::vector <double> y);
};

#include "Utils.h"

#include <dirent.h>
#include <stdlib.h> // for EXIT_FAILURE
#include <string.h>
#include <fstream>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!

#include <limits.h>
#include <string>
#include <ctype.h>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

// TODO: Delete getStDists (copy) files

typedef boost::unordered_map <long long, int> umap;
typedef boost::unordered_map <long long, int>::iterator umap_Itr;
typedef boost::unordered_set <long long> uset;
typedef boost::unordered_set <long long>::iterator uset_Itr;

class Ways 
{
    protected:
        std::string _dirName;
        const std::string _city;
    public:
        int err;
        umap nodeMap;
        uset terminalNodes;
        std::vector <std::pair <double, double> > latlons;
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
};

/************************************************************************
 * This software is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 * 
 * <mainStnDists.c++>
 * Calculates pair-wise distances between points routed through and OSM network
 * using internal preference weightings for highway kinds.
 * 
 * Author: Mark Padgham, April 2015
 ************************************************************************/

/*
 * Stores a planet-osm file as a boost::graph. The latter rely on property maps,
 * which are statically-typed entities, and so data are initially read and
 * stored as unordered_maps to be subsequently read into the graph. This initial
 * storage as unordered_maps also speeds up mapping node IDs in ways onto
 * lat-lon coordinates.
 *
 * The boost::graphs are constructed in two phases:
 * (1) A graph "gFull" is constructed from all OSM nodes and highways, and used
 * to identify both the largest connected component and all terminal or junction
 * nodes (simply as those that appear in multiple different ways). The bike
 * stations are then mapped onto the nearest vertices within this largest
 * component, and these vertices are also added to terminalNodes.
 * (2) A second reading of the data is used to make "gCompact" which only has
 * terminalNodes as vertices, and edge distances as traced along all
 * intermediate nodes.
 *
 * TODO: CGAL check that station points lie within OSM polygon
 */

#include "Utils.h"

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/connected_components.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/bzip2.hpp>


typedef std::pair <double, double> ddPair;

typedef boost::unordered_map <long long, int> umapInt;
typedef boost::unordered_map <long long, int>::iterator umapInt_Itr;
typedef boost::unordered_map <long long, ddPair> umapPair;
typedef boost::unordered_map <long long, ddPair>::iterator umapPair_Itr;

typedef std::pair <std::string, float> ProfilePair;
typedef float Weight;

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

// edge and vertex bundles for the boost::graph
struct bundled_edge_type
{ 
    Weight weight; // weighted distance
    float dist;
};

struct bundled_vertex_type
{
    long long id;
    std::string name;
    float lat, lon;
    int component;
};

/*
 * VertMap can be used to filter the largest connected component, but a filtered
 * graph retains all vertices regardless, so there's no real advantage here to
 * filtering.
 */
template <typename VertMap>
struct in_component_0 {
    in_component_0 () { }
    in_component_0 (VertMap comp0) : m_comp0(comp0) { }
    template <typename Vertex>
        bool operator()(const Vertex& v) const {
            return boost::get (m_comp0, v) == 0; // largest connected has id=0
        }
    VertMap m_comp0;
};

class Ways 
{
    using Graph_t = boost::adjacency_list< boost::vecS, boost::vecS, 
          boost::directedS, bundled_vertex_type, bundled_edge_type >;

    using Vertex = boost::graph_traits<Graph_t>::vertex_descriptor;

    private:
        Graph_t gFull, gCompact;
    protected:
        float latmin, lonmin, latmax, lonmax;
        std::string _dirName;
        const std::string _city;
        const std::string osmDir = "/data/data/bikes/";
        std::string osmFile;
        std::vector <ProfilePair> profile;
        boost::unordered_set <long long> terminalNodes;
        dmat distMat;

    public:
        int err, count;
        long long node;
        float d;
        std::string tempstr;


        /*
         * The storage of nodeNames is done in readNodes, while they are then
         * only subsequently used to replace the long long OSM numbers with
         * corresponding ints when storing the boost::graph in the readWays
         * routine.
         */
        umapPair allNodes;
        umapInt nodeNames;
        std::vector <Station> stationList;
        std::vector <float> dists;
        std::vector <bool> idDone; // TODO: DELETE!
        Ways (std::string str)
            : _city (str)
        {
            tempstr = _city;
            std::transform (tempstr.begin(), tempstr.end(), 
                    tempstr.begin(), ::toupper);
            std::cout << "---" << tempstr << "---" << std::endl;
            osmFile = osmDir + "planet-" + _city + ".osm";
            
            setProfile ();

            err = getBBox ();
            err = readNodes();
            err = readAllWays ();
            err = getConnected ();
            err = readStations ();
            distMat.resize (stationList.size (), stationList.size ());

            // gFull is no longer needed, so can be destroyed
            gFull.clear ();
            
            err = readCompactWays ();
            err = remapStations ();

            std::cout << "Getting inter-station distances";
            std::cout.flush ();
            count = 0;
            idDone.resize (stationList.size ());
            for (int i=0; i<stationList.size (); i++)
                idDone [i] = false;
            for (std::vector<Station>::iterator itr=stationList.begin();
                    itr != stationList.end(); itr++)
            {
                err = dijkstra (itr->nodeIndex);
                assert (dists.size () == stationList.size ());
                std::cout << "\rGetting inter-station distances " <<
                    count << "/" << stationList.size () << " ";
                std::cout.flush ();
                count++;
            }
            std::cout << "\rGetting inter-station distances " <<
                stationList.size () << "/" << stationList.size () <<
                " done." << std::endl;

            for (int i=0; i<stationList.size (); i++)
                if (!idDone [i])
                    std::cout << "ERROR: ID#" << i << " was not done" <<
                        std::endl;

            writeDMat ();
        }
        ~Ways ()
        {
            stationList.resize (0);
            dists.resize (0);
        }

        std::string returnDirName () { return _dirName; }
        std::string returnCity () { return _city;   }

        void setProfile ()
        {
            profile.resize (0);
            // Note that routino has motorway preference = 0.0, but this doesn't
            // work if weights for preference=0.0 are set to FLOAT_MAX.
            profile.push_back (std::make_pair ("motorway", 0.01));
            profile.push_back (std::make_pair ("trunk", 0.3));
            profile.push_back (std::make_pair ("primary", 0.7));
            profile.push_back (std::make_pair ("secondary", 0.8));
            profile.push_back (std::make_pair ("tertiary", 0.9));
            profile.push_back (std::make_pair ("unclassified", 0.9));
            profile.push_back (std::make_pair ("residential", 0.9));
            profile.push_back (std::make_pair ("service", 0.9));
            profile.push_back (std::make_pair ("track", 0.9));
            profile.push_back (std::make_pair ("cycleway", 1.0));
            profile.push_back (std::make_pair ("path", 0.9));
            profile.push_back (std::make_pair ("steps", 0.1));
            profile.push_back (std::make_pair ("ferry", 0.2));
            profile.push_back (std::make_pair ("footway", 0.5));
        };

        int getBBox ();
        int readNodes ();
        int readTerminalNodes ();
        int readAllWays ();
        int getConnected ();
        int readStations ();
        int remapStations ();
        int readCompactWays ();
        int dijkstra (long long fromNode);
        int writeDMat ();

        float calcDist (std::vector <float> x, std::vector <float> y);
        long long nearestNode (float lon0, float lat0);
        std::string getCity () { return _city;  }
};

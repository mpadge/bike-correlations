#include "mainStnDists.h"

int main(int argc, char *argv[]) {
    std::string city = "boston";

    Ways ways(city);
};

int Ways::readNodes ()
{
    int ipos [3];
    long long node;
    float lat, lon;
    std::string linetxt, txt, fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 

    std::cout << "Reading nodes ...";
    std::cout.flush ();

    while (getline (in_file, linetxt, '\n'))
    {
        ipos [0] = linetxt.find ("<node id=");
        ipos [1] = linetxt.find ("lat=");
        if (ipos [0] != std::string::npos && ipos [1] != std::string::npos)
        {
            ipos [2] = linetxt.find (" version=") - ipos [0] - 11;
            node = atoll (linetxt.substr (ipos [0] + 10, ipos [2]).c_str());
            linetxt = linetxt.substr (ipos [1] + 5, linetxt.length () - ipos [1] - 1);
            ipos [0] = linetxt.find ("\" lon=");
            lat = atof (linetxt.substr (0, ipos [0]).c_str ());
            ipos [0] = linetxt.find ("lon=");
            lon = atof (linetxt.substr (ipos [0] + 5, 
                        linetxt.length () - ipos [0] - 5).c_str ());

            allNodes [node] = std::make_pair (lat, lon);
        }
    } // end while getline
    in_file.close ();

    std::cout << "\rRead coordinates of " << allNodes.size () << " nodes." <<
        std::endl;

    return (allNodes.size ());
} // end Way::readNodes

int Ways::readWays ()
{
    bool inway = false, highway = false;
    int ipos, id0, id1, nodeCount = 0;
    long long node;
    float lat0, lon0, lat1, lon1;
    umapPair_Itr umapitr;
    Segment seg;
    std::vector <long long> waynodes, ids;
    std::vector <float> lats, lons;
    std::string linetxt, tempstr,
        fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;
    std::ofstream out_file; // TODO: DELETE!
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 

    wayList.resize (0);

    std::cout << "Reading ways ...";
    std::cout.flush ();

    out_file.open ("map.csv");

    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
        {
            inway = true;
            highway = false;
            waynodes.resize (0);
            ids.resize (0);
        }
        else if (linetxt.find ("</way>") != std::string::npos)
        {
            //if (highway && waynodes [0] != waynodes [waynodes.size () - 1])
            if (highway)
            {
                assert (allNodes.find (waynodes[0]) != allNodes.end());
                node = (*waynodes.begin());
                lat0 = (((*allNodes.find(node)).second).first);
                lon0 = (((*allNodes.find(node)).second).second);
                if (nodeNames.find (node) == nodeNames.end())
                {
                    id0 = nodeCount;
                    nodeNames [node] = id0;
                    nodeCount++;
                }
                else
                    id0 = (*nodeNames.find (node)).second;
                // The following iterator uses std::next (c++11) to start loop
                // at 1 rather than 0.
                for (std::vector <long long>::iterator 
                        itr=std::next (waynodes.begin());
                        itr != waynodes.end(); itr++)
                {
                    assert ((umapitr = allNodes.find (*itr)) != allNodes.end());
                    lat1 = ((*umapitr).second).first;
                    lon1 = ((*umapitr).second).second;
                    node = (*itr);
                    if (nodeNames.find (node) == nodeNames.end())
                    {
                        id1 = nodeCount;
                        nodeNames [node] = id1;
                        nodeCount++;
                    }
                    else
                        id1 = (*nodeNames.find (node)).second;
                    seg.from = id0;
                    seg.to = id1;
                    seg.d = calcDist (lon0, lat0, lon1, lat1);
                    wayList.push_back (seg);

                    seg.from = id1;
                    seg.to = id0;
                    wayList.push_back (seg);
                    
                    id0 = id1;
                    lat0 = lat1;
                    lon0 = lon1;

                    out_file << lon0 << "," << lat0 << "," << lon1 <<
                        "," << lat1 << std::endl;
                }
            } // end if highway
            inway = false;
        } // end else if end way
        else if (inway)
        {
            /*
             * Nodes have to first be stored, because they are only subsequently
             * analysed if they are part of a highway and do not loop. 
             */
            if (linetxt.find ("<nd") != std::string::npos)
            {
                ipos = linetxt.find ("<nd ref=");
                linetxt = linetxt.substr (ipos + 9, 
                        linetxt.length () - ipos - 9);
                waynodes.push_back (atoll (linetxt.c_str ()));
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
                highway = true;
        } // end else if inway
    } // end while getline
    in_file.close ();
    out_file.close ();

    /*
     * nodes and corresponding indices can then be respectively obtained with
     * node = nodeNames.find (node)->first; // redundant
     * index = nodeNames.find (node)->second;
     */

    std::cout << "\rRead " << wayList.size() << " ways with " << 
        nodeNames.size () << " unique nodes." << std::endl;

    return 0;
} // end function readWays


float Ways::calcDist (float x0, float y0, float x1, float y1)
{
    float d, xd, yd;

    xd = (x1 - x0) * PI / 180.0;
    yd = (y1 - y0) * PI / 180.0;
    d = sin (yd / 2.0) * sin (yd / 2.0) + cos (y1 * PI / 180.0) *
        cos (y0 * PI / 180.0) * sin (xd / 2.0) * sin (xd / 2.0);
    d = 2.0 * atan2 (sqrt (d), sqrt (1.0 - d));
    d = d * 6371.0;

    return d; // in kilometres!
} // end function calcDist


int Ways::readStations ()
{
    int ipos = 0;
    float lat, lon;
    // hubway_stations is extracted directly from the zip file of hubway data
    std::string linetxt, txt, fname = "data/hubway_stations.csv";
    std::ifstream in_file;
    Station station;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n'); // header
    while (getline (in_file, linetxt, '\n'))
        ipos++;
    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n'); // header

    stationList.resize (0);
    std::cout << "Matching " << ipos << " stations to nearest OSM nodes ...";
    std::cout.flush ();

    while (getline (in_file, linetxt, '\n'))
    {
        for (int i=0; i<4; i++)
        {
            ipos = linetxt.find (",");
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (",");
        station.lat = atof (linetxt.substr (0, ipos).c_str());
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (",");
        station.lon = atof (linetxt.substr (0, ipos).c_str());
        station.node = nearestNode (station.lon, station.lat);
        assert (nodeNames.find (station.node) != nodeNames.end());
        station.nodeIndex = nodeNames.find (station.node)->second;
        stationList.push_back (station);
    } // end while getline
    in_file.close ();
    std::cout << " done." << std::endl;

    return stationList.size ();
}


long long Ways::nearestNode (float lon0, float lat0)
{
    int id;
    long long node = -9999;
    float d, lon, lat, xd, yd, dmin = FLOAT_MAX;

    /*
     * This is the only place in the whole program that requires explicit
     * looping over nodes, and takes longer than all other bits, so distances to
     * nearest nodes are simply calcualted as sums of the 2 abs distances in
     * lats & lons (even though this saves only about 3.5s!).
     *
     * nodeNames lists the nodes in non-circuluar highways, which must be
     * matched to the full list of allNodes to find corresponding lats&lons.
     */

    for (umapInt_Itr itr = nodeNames.begin(); itr != nodeNames.end (); itr++)
    {
        lat = ((*allNodes.find ((*itr).first)).second).first;
        lon = ((*allNodes.find ((*itr).first)).second).second;
        d = std::abs (lon - lon0) + std::abs (lat - lat0);
        /*
        xd = (lon - lon0) * PI / 180.0;
        yd = (lat - lat0) * PI / 180.0;
        d = sin (yd / 2.0) * sin (yd / 2.0) + cos (lat * PI / 180.0) *
            cos (lat0 * PI / 180.0) * sin (xd / 2.0) * sin (xd / 2.0);
        d = 2.0 * atan2 (sqrt (d), sqrt (1.0 - d));
        d = d * 6371.0;
        */
        if (d < dmin)
        {
            dmin = d;
            node = (*itr).first;
        }
    }

    return node;
} // end function calcDist


int Ways::sp (long long fromNode)
{
    int nvalid = 0;
    // Largely adapted from the boost example and:
    // http://programmingexamples.net/wiki/Boost/BGL/DijkstraComputePath
    typedef float Weight;
    typedef boost::property <boost::edge_weight_t, Weight> WeightProperty;
    typedef boost::property <boost::vertex_name_t, int> NameProperty;

    typedef boost::adjacency_list < boost::listS, boost::vecS, boost::directedS,
            NameProperty, WeightProperty > Graph;

    typedef boost::graph_traits < Graph >::vertex_descriptor Vertex;

    typedef boost::property_map < Graph, boost::vertex_index_t >::type IndexMap;
    typedef boost::property_map < Graph, boost::vertex_name_t >::type NameMap;

    typedef boost::iterator_property_map 
                        < Vertex*, IndexMap, Vertex, Vertex& > PredecessorMap;
    typedef boost::iterator_property_map 
                        < Weight*, IndexMap, Weight, Weight& > DistanceMap;
    typedef boost::iterator_property_map 
                        < Vertex*, IndexMap, Vertex, Vertex& > ComponentMap;

    Graph g;

    for (umapInt_Itr itr = nodeNames.begin(); itr != nodeNames.end(); itr++)
        boost::add_vertex ((*itr).second, g);
    for (std::vector<Segment>::iterator itr = wayList.begin();
            itr != wayList.end(); itr++)
        boost::add_edge ((*itr).from, (*itr).to, (*itr).d, g);

    // Check whether graph is connected:
    std::vector<int> component(num_vertices(g));
    int num = boost::connected_components(g, &component[0]);
    std::cout << "Graph has " << num << " connected components." << std::endl;

    std::vector<Vertex> predecessors (boost::num_vertices(g)); 
    std::vector<Weight> distances (boost::num_vertices(g)); 

    IndexMap indexMap = boost::get (boost::vertex_index, g);
    PredecessorMap predecessorMap (&predecessors[0], indexMap);
    DistanceMap distanceMap (&distances[0], indexMap);

    // predecessor and distance maps can be passed in any order
    int start = vertex (fromNode, g);
    boost::dijkstra_shortest_paths (g, start, 
            boost::distance_map (distanceMap).predecessor_map (predecessorMap));


    /*
    // Trace back from end node
    typedef std::vector<Graph::edge_descriptor> PathType;
    PathType path;

    int count = 0;
    for (std::vector<Weight>::iterator itr=distances.begin();
            itr != distances.end(); itr++)
        if ((*itr) == FLOAT_MAX)
            count++;
    std::cout << count << " / " << distances.size () <<
        " nodes not reachable." << std::endl;

    Vertex v = toNode;
    count = 0;
    for (Vertex u = predecessorMap[v]; u != v; v = u, u = predecessorMap[v]) 
    {
        std::pair<Graph::edge_descriptor, bool> edgePair = boost::edge(u, v, g);
        Graph::edge_descriptor edge = edgePair.first;
        path.push_back ( edge );
        count++;
    }
    */

    dists.resize (0);
    for (std::vector <Station>::iterator itr = stationList.begin();
            itr != stationList.end (); itr++)
    {
        dists.push_back (distances [(*itr).nodeIndex]);
        if (distances [(*itr).nodeIndex] < FLOAT_MAX)
            nvalid++;
    }

    // Write shortest path
    /*
    std::cout << "Shortest path from " << start << " to " << v0 << ":" << std::endl;
    NameMap nameMap = boost::get (boost::vertex_name, g);
    float totalDistance = 0;
    for (PathType::reverse_iterator pathIterator = path.rbegin(); 
            pathIterator != path.rend(); ++pathIterator)
    {
        std::cout << "v" << nameMap [boost::source (*pathIterator, g)] << 
            " -> v" << nameMap [boost::target (*pathIterator, g)] <<
            " = " << boost::get (boost::edge_weight, g, *pathIterator) << 
            std::endl;
    }
    */
    return (nvalid);
}

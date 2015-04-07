#include "mainStnDists.h"

int main(int argc, char *argv[]) {
    std::string city = "boston";

    Ways ways(city);
};

int Ways::readNodes ()
{
    int ipos [3];
    long long id;
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
            id = atoll (linetxt.substr (ipos [0] + 10, ipos [2]).c_str());
            linetxt = linetxt.substr (ipos [1] + 5, linetxt.length () - ipos [1] - 1);
            ipos [0] = linetxt.find ("\" lon=");
            lat = atof (linetxt.substr (0, ipos [0]).c_str ());
            ipos [0] = linetxt.find ("lon=");
            lon = atof (linetxt.substr (ipos [0] + 5, 
                        linetxt.length () - ipos [0] - 5).c_str ());

            allNodes [id] = std::make_pair (lat, lon);
        }
    } // end while getline
    in_file.close ();

    std::cout << "\rRead coordinates of " << allNodes.size () << " nodes." <<
        std::endl;

    return (allNodes.size ());
} // end Way::readNodes


int Ways::readTerminalNodes ()
{
    bool inway = false, highway = false, inList;
    int ipos, nodeCount = 0;
    long long startNode, endNode;
    std::string linetxt, fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;

    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 

    std::cout << "Reading terminal nodes of ways ...";
    std::cout.flush ();

    startNode = endNode = INT_MIN;

    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
            inway = true;
        else if (linetxt.find ("</way>") != std::string::npos)
        {
            inway = false;
            if (highway && startNode != endNode)
            {
                if (nodeNames.find (startNode) == nodeNames.end())
                {
                    nodeNames [startNode] = nodeCount;
                    nodeCount++;
                }
                if (nodeNames.find (endNode) == nodeNames.end())
                {
                    nodeNames [endNode] = nodeCount;
                    nodeCount++;
                }
            } // end if highway
            highway = false;
            startNode = INT_MIN;
        } // end else if end way
        else if (inway)
        {
            if (linetxt.find ("<nd") != std::string::npos)
            {
                ipos = linetxt.find ("<nd ref=");
                linetxt = linetxt.substr (ipos + 9, 
                        linetxt.length () - ipos - 9);
                if (startNode < 0)
                    startNode = atoll (linetxt.c_str ());
                else
                    endNode = atoll (linetxt.c_str ());
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
                highway = true;
        } // end else if inway
    } // end while getline
    in_file.close ();

    std::cout << "\rRead " << nodeNames.size () << 
        " terminal nodes of ways." << std::endl;

    return nodeNames.size ();
} // end Ways::readTerminalNodes

int Ways::readWays ()
{
    bool inway = false, highway = false;
    int ipos;
    float dist;
    long long node; 
    umapPair_Itr umapitr;
    segment seg;
    std::vector <long long> waynodes, ids;
    std::vector <float> lats, lons;
    std::string linetxt, tempstr,
        fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 

    wayList.resize (0);

    std::cout << "Reading ways ...";
    std::cout.flush ();

    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
        {
            inway = true;
            waynodes.resize (0);
            ids.resize (0);
        }
        else if (linetxt.find ("</way>") != std::string::npos)
        {
            inway = false;
            if (highway && waynodes [0] != waynodes [waynodes.size () - 1])
            {
                lats.resize (0);
                lons.resize (0);
                for (std::vector <long long>::iterator itr=waynodes.begin();
                        itr != waynodes.end(); itr++)
                {
                    if ((umapitr = allNodes.find (*itr)) != allNodes.end())
                    {
                        lats.push_back (((*umapitr).second).first);
                        lons.push_back (((*umapitr).second).second);
                    }
                    else // should not happen!
                        std::cout << "Node not found" << std::endl;
                    /*
                     * If a single way crosses a terminal node, then break it
                     * into two separate ways either side thereof.
                     */
                    if (itr > waynodes.begin() &&
                            (nodeNames.find (*itr)) != nodeNames.end())
                    {
                        dist = calcDist (lons, lats);
                        seg.from = (*nodeNames.find (ids [0])).second;
                        seg.to = (*nodeNames.find(*itr)).second;
                        seg.d = dist;
                        wayList.push_back (seg);
                        ids.resize (0);
                        ids.push_back (*itr);
                        lats.resize (0);
                        lons.resize (0);
                        lats.push_back (((*umapitr).second).first);
                        lons.push_back (((*umapitr).second).second);
                    }
                }
                //assert (lats.size () == waynodes.size ());
                highway = false;
            } // end if highway
            highway = false;
        } // end else if end way
        else if (inway)
        {
            /*
             * Nodes have to first be stored, because they are only subsequently
             * analysed if they do not loop. waynodes provides the reference
             * list, while ids are broken into segments at any terminal nodes.
             */
            if (linetxt.find ("<nd") != std::string::npos)
            {
                ipos = linetxt.find ("<nd ref=");
                linetxt = linetxt.substr (ipos + 9, 
                        linetxt.length () - ipos - 9);
                node = atoll (linetxt.c_str ());
                waynodes.push_back (node);
                ids.push_back (node);
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
                highway = true;
        } // end else if inway
    } // end while getline
    in_file.close ();

    std::cout << "\rRead " << wayList.size() << " ways" << std::endl;

    return 0;
} // end function readWays


float Ways::calcDist (std::vector <float> x, std::vector <float> y)
{
    float d, dsum = 0.0, x0, y0, x1 = x[0], y1 = y[0], xd, yd;
    assert (x.size () == y.size ());

    for (int i=1; i<x.size (); i++)
    {
        x0 = x1;
        y0 = y1;
        x1 = x[i];
        y1 = y[i];
        xd = (x1 - x0) * PI / 180.0;
        yd = (y1 - y0) * PI / 180.0;
        d = sin (yd / 2.0) * sin (yd / 2.0) + cos (y1 * PI / 180.0) *
            cos (y0 * PI / 180.0) * sin (xd / 2.0) * sin (xd / 2.0);
        d = 2.0 * atan2 (sqrt (d), sqrt (1.0 - d));
        dsum += d * 6371.0;
    }

    return dsum; // in kilometres!
} // end function calcDist


float Ways::sp (long long fromNode)
{
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

    Graph g;

    for (umapInt_Itr itr = nodeNames.begin(); itr != nodeNames.end(); itr++)
        boost::add_vertex ((*itr).second, g);
    int minfrom = 999999, minto = 999999;
    for (std::vector<segment>::iterator itr = wayList.begin();
            itr != wayList.end(); itr++)
        boost::add_edge ((*itr).from, (*itr).to, (*itr).d, g);

    std::vector<Vertex> predecessors (boost::num_vertices(g)); 
    std::vector<Weight> distances (boost::num_vertices(g)); 

    IndexMap indexMap = boost::get (boost::vertex_index, g);
    PredecessorMap predecessorMap (&predecessors[0], indexMap);
    DistanceMap distanceMap (&distances[0], indexMap);

    // predecessor and distance maps can be passed in any order
    int start = vertex (0, g);
    boost::dijkstra_shortest_paths (g, start, 
            boost::distance_map (distanceMap).predecessor_map (predecessorMap));

    // Trace back from end node
    typedef std::vector<Graph::edge_descriptor> PathType;
    PathType path;

    Vertex v0 = 1; // Node from which to start traceback
    Vertex v = v0;
    for (Vertex u = predecessorMap[v]; u != v; v = u, u = predecessorMap[v]) 
    {
        std::pair<Graph::edge_descriptor, bool> edgePair = boost::edge(u, v, g);
        Graph::edge_descriptor edge = edgePair.first;
        path.push_back ( edge );
    }

    // Write shortest path
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
    std::cout << "Distance: " << distanceMap[v0] << std::endl;
}


float Ways::spOld (long long fromNode)
{
    std::cout << "fromNode = " << fromNode << std::endl;


    // Largely adapted from the boost example and:
    // http://programmingexamples.net/wiki/Boost/BGL/DijkstraComputePath
    typedef float Weight;
    typedef boost::property <boost::edge_weight_t, Weight> WeightProperty;
    typedef boost::property <boost::vertex_name_t, long long> NameProperty;

    typedef boost::adjacency_list < boost::listS, boost::vecS, boost::directedS,
            NameProperty, WeightProperty > Graph;

    typedef boost::graph_traits < Graph >::vertex_descriptor Vertex;

    typedef boost::property_map < Graph, boost::vertex_index_t >::type IndexMap;
    typedef boost::property_map < Graph, boost::vertex_name_t >::type NameMap;

    typedef boost::iterator_property_map 
                        < Vertex*, IndexMap, Vertex, Vertex& > PredecessorMap;
    typedef boost::iterator_property_map 
                        < Weight*, IndexMap, Weight, Weight& > DistanceMap;

    typedef std::pair<std::string, std::string> Edge;

    Graph g;

    boost::add_vertex (0, g);
    boost::add_vertex (1, g);
    boost::add_vertex (2, g);
    boost::add_vertex (3, g);
    boost::add_vertex (4, g);
    boost::add_edge (0, 2, 1.1, g);
    boost::add_edge (1, 1, 2.2, g);
    boost::add_edge (1, 2, 1.1, g);
    boost::add_edge (1, 3, 2.2, g);
    boost::add_edge (2, 1, 7.7, g);
    boost::add_edge (2, 3, 4.4, g);
    boost::add_edge (3, 2, 3.3, g);
    boost::add_edge (3, 4, 1.1, g);
    boost::add_edge (4, 0, 1.1, g);
    boost::add_edge (4, 1, 1.1, g);

    std::vector<Vertex> predecessors (boost::num_vertices(g)); 
    std::vector<Weight> distances (boost::num_vertices(g)); 

    IndexMap indexMap = boost::get (boost::vertex_index, g);
    PredecessorMap predecessorMap (&predecessors[0], indexMap);
    DistanceMap distanceMap (&distances[0], indexMap);

    // predecessor and distance maps can be passed in any order
    int start = vertex (0, g);
    boost::dijkstra_shortest_paths (g, start, 
            boost::distance_map (distanceMap).predecessor_map (predecessorMap));

    // Trace back from end node
    typedef std::vector<Graph::edge_descriptor> PathType;
    PathType path;

    Vertex v0 = 4; // Node from which to start traceback
    Vertex v = v0;
    for (Vertex u = predecessorMap[v]; u != v; v = u, u = predecessorMap[v]) 
    {
        std::pair<Graph::edge_descriptor, bool> edgePair = boost::edge(u, v, g);
        Graph::edge_descriptor edge = edgePair.first;
        path.push_back ( edge );
    }

    // Write shortest path
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
    std::cout << "Distance: " << distanceMap[v0] << std::endl;
}

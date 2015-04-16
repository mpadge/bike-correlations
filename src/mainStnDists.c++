#include "mainStnDists.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                                MAIN                                **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int main(int argc, char *argv[]) {
    std::string city = "boston";

    Ways ways(city);
};


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                              READNODES                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int Ways::readNodes ()
{
    int ipos [3];
    long long node;
    float lat, lon;
    std::string linetxt, txt;
    std::ifstream in_file;
    
    in_file.open (osmFile.c_str (), std::ifstream::in);
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
    } 
    in_file.close ();

    std::cout << "\rRead coordinates of " << allNodes.size () << " nodes." <<
        std::endl;

    return (allNodes.size ());
} // end Way::readNodes


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            READALLWAYS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int Ways::readAllWays ()
{
    bool inway = false, highway = false, oneway;
    int ipos, id0, id1, nodeCount = 0, nways = 0;
    long long node;
    float d, lat0, lon0, lat1, lon1, weight;
    umapPair_Itr umapitr;
    boost::unordered_set <long long> nodeList;
    std::vector <long long> waynodes, ids;
    std::vector <float> lats, lons;
    std::string linetxt, tempstr, highwayType;
    std::ifstream in_file;
    
    in_file.open (osmFile.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 

    // oneways should only be "yes", but wiki allows the other two as well
    typedef std::vector <std::string> strvec;
    strvec oneWayList;
    oneWayList.push_back ("k=\"oneway\" v=\"yes");
    oneWayList.push_back ("k=\"oneway\" v=\"0");
    oneWayList.push_back ("k=\"oneway\" v=\"true");

    //wayList.resize (0);

    /*
     * The boost::graph is only given size through the following vertex
     * additions. Vertex numbers are implicit and sequential, enumeratred by
     * nodeCount, and edges must reference these numbers.
     */
    bundled_vertex_type oneVert;
    bundled_edge_type oneEdge;

    std::cout << "Reading ways ...";
    std::cout.flush ();

    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
        {
            inway = true;
            highway = false;
            oneway = false;
            weight = -9999.0;
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
                // Any highway nodes that appear more than once are put onto
                // terminalNodes
                if (nodeList.find (node) == nodeList.end())
                    nodeList.insert (node);
                else if (terminalNodes.find (node) == terminalNodes.end())
                    terminalNodes.insert (node);
                lat0 = (((*allNodes.find(node)).second).first);
                lon0 = (((*allNodes.find(node)).second).second);
                if (nodeNames.find (node) == nodeNames.end())
                {
                    id0 = nodeCount;
                    nodeNames [node] = id0;
                    nodeCount++;
                    oneVert.id = node;
                    oneVert.lat = lat0;
                    oneVert.lon = lon0;
                    boost::add_vertex (oneVert, gFull);
                }
                else
                    id0 = (*nodeNames.find (node)).second;
                // Note std::next (c++11) in loop
                for (std::vector <long long>::iterator 
                        itr=std::next (waynodes.begin());
                        itr != waynodes.end(); itr++)
                {
                    assert ((umapitr = allNodes.find (*itr)) != allNodes.end());
                    lat1 = ((*umapitr).second).first;
                    lon1 = ((*umapitr).second).second;
                    node = (*itr);

                    if (nodeList.find (node) == nodeList.end())
                        nodeList.insert (node);
                    else if (terminalNodes.find (node) == terminalNodes.end())
                        terminalNodes.insert (node);

                    if (nodeNames.find (node) == nodeNames.end())
                    {
                        id1 = nodeCount;
                        nodeNames [node] = id1;
                        nodeCount++;
                        oneVert.id = node;
                        oneVert.lat = lat1;
                        oneVert.lon = lon1;
                        boost::add_vertex (oneVert, gFull);
                    }
                    else
                        id1 = (*nodeNames.find (node)).second;
                    d = calcDist ({lon0, lon1}, {lat0, lat1});
                    if (weight == 0.0)
                        oneEdge.weight = FLOAT_MAX;
                    else
                        oneEdge.weight = d / weight;
                    oneEdge.dist = d;
                    boost::add_edge(id0, id1, oneEdge, gFull);
                    if (!oneway)
                        boost::add_edge(id1, id0, oneEdge, gFull);
                    nways++;

                    id0 = id1;
                    lat0 = lat1;
                    lon0 = lon1;
                }
            } // end if highway
            inway = false;
        } // end else if end way
        else if (inway)
        {
            /*
             * Nodes have to first be stored, because they are only subsequently
             * analysed if they are part of a highway.
             */
            if (linetxt.find ("<nd") != std::string::npos)
            {
                ipos = linetxt.find ("<nd ref=");
                linetxt = linetxt.substr (ipos + 9, 
                        linetxt.length () - ipos - 9);
                node = atoll (linetxt.c_str ());
                waynodes.push_back (atoll (linetxt.c_str ()));
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
            {
                // highway is only true if it has one of the values listed in
                // the profile
                for (std::vector<ProfilePair>::iterator itr = profile.begin();
                        itr != profile.end(); itr++)
                {
                    tempstr = "v=\"" + (*itr).first;
                    if (linetxt.find (tempstr) != std::string::npos)
                    {
                        highway = true;
                        highwayType = (*itr).first;
                        weight = (*itr).second;
                        break;
                    }
                    for (strvec::iterator oit = oneWayList.begin();
                            oit != oneWayList.end(); oit++)
                        if (linetxt.find (*oit) != std::string::npos)
                            oneway = true;
                }
            }
        } // end else if inway
    } // end while getline
    in_file.close ();
    oneWayList.resize (0);

    /*
     * nodes and corresponding indices can then be respectively obtained with
     * node = nodeNames.find (node)->first; // redundant
     * index = nodeNames.find (node)->second;
     */

    std::cout << "\rRead " << nways << " ways with " << 
        nodeNames.size () << " unique nodes." << std::endl;

    return 0;
} // end function readAllWays


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                              CALCDIST                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/


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


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            GETCONNECTED                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int Ways::getConnected ()
{
    std::vector<int> compvec(num_vertices(gFull));
    int num = boost::connected_components(gFull, &compvec[0]);

    // Then store component info in vertices
    /*
    typedef boost::graph_traits <Graph_t>::vertex_iterator viter;
    std::pair <viter, viter> vp;
    for (vp = vertices(gFull); vp.first != vp.second; ++vp.first)
        vertex_component [*vp.first] = compvec [*vp.first];
     */
    // Alternative:
    boost::property_map< Graph_t, int bundled_vertex_type::* >::type 
        vertex_component = boost::get(&bundled_vertex_type::component, gFull);
    auto vs = boost::vertices (gFull);
    for (auto vit = vs.first; vit != vs.second; ++vit)
        vertex_component [*vit] = compvec [*vit];

    // Optional filtering of component = 0:
    /*
    in_component_0 <VertMap> filter (boost::get 
        (&bundled_vertex_type::component, gFull));
    boost::filtered_graph <Graph, in_component_0 <VertMap> > fg (g, filter);
    // Next lines reveal filtering does not actually reduce the graph
    auto vsfg = boost::vertices (fg);
    for (auto vit = vsfg.first; vit != vsfg.second; ++vit)
        std::cout << *vit << std::endl;
     */
    return (num);
} // end function getConnected


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            READSTATIONS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/


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

    /*
     * the main task of this routine is to assign stations to their nearest
     * highway nodes. These nodes must be within the largest connected component
     * of the OSM graph, which boost *seems* always seems to number with 0.
     */
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


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            NEARESTNODE                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/


long long Ways::nearestNode (float lon0, float lat0)
{
    int id;
    long long node = -9999;
    float d, lon, lat, xd, yd, dmin = FLOAT_MAX;

    /*
     * This is the only place in the whole program that requires explicit
     * looping over vertices, and takes longer than all other bits. Calculating
     * distances to nearest vertices as sums of the 2 abs distances in lats &
     * lons saves only about 2.5s.
     *
     * nodeNames lists the highway vertices, which must be matched to the full
     * list of allNodes to find corresponding lats&lons.
     */

    boost::property_map< Graph_t, long long bundled_vertex_type::* >::type 
        vertex_id = boost::get(&bundled_vertex_type::id, gFull);
    boost::property_map< Graph_t, float bundled_vertex_type::* >::type 
        vertex_lat = boost::get(&bundled_vertex_type::lat, gFull);
    boost::property_map< Graph_t, float bundled_vertex_type::* >::type 
        vertex_lon = boost::get(&bundled_vertex_type::lon, gFull);
    boost::property_map< Graph_t, int bundled_vertex_type::* >::type 
        vertex_component = boost::get(&bundled_vertex_type::component, gFull);

    auto vs = boost::vertices (gFull);
    for (auto vit = vs.first; vit != vs.second; ++vit)
    {
        if (vertex_component [*vit] == 0)
        {
            lat = vertex_lat [*vit];
            lon = vertex_lon [*vit];
            //d = std::abs (lon - lon0) + std::abs (lat - lat0);
            xd = (lon - lon0) * PI / 180.0;
            yd = (lat - lat0) * PI / 180.0;
            d = sin (yd / 2.0) * sin (yd / 2.0) + cos (lat * PI / 180.0) *
                cos (lat0 * PI / 180.0) * sin (xd / 2.0) * sin (xd / 2.0);
            d = 2.0 * atan2 (sqrt (d), sqrt (1.0 - d));
            d = d * 6371.0;
            if (d < dmin)
            {
                dmin = d;
                node = vertex_id [*vit];
            }
        }
    }

    return node;
} // end function calcDist


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                              DIJKSTRA                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int Ways::dijkstra (long long fromNode)
{
    std::vector<Vertex> predecessors (boost::num_vertices(gFull)); 
    std::vector<Weight> distances (boost::num_vertices(gFull)); 

    boost::property_map< Graph_t, long long bundled_vertex_type::* >::type 
        vertex_id = boost::get(&bundled_vertex_type::id, gFull);
    boost::property_map< Graph_t, float bundled_vertex_type::* >::type 
        vertex_lat = boost::get(&bundled_vertex_type::lat, gFull);
    boost::property_map< Graph_t, float bundled_vertex_type::* >::type 
        vertex_lon = boost::get(&bundled_vertex_type::lon, gFull);

    auto p_map = boost::make_iterator_property_map
        (&predecessors[0], boost::get(boost::vertex_index, gFull));
    auto d_map = boost::make_iterator_property_map
        (&distances[0], boost::get(boost::vertex_index, gFull));
    auto w_map = boost::get(&bundled_edge_type::weight, gFull); 
    
    int start = vertex (fromNode, gFull);
    boost::dijkstra_shortest_paths(gFull, start,
            weight_map(w_map). 
            predecessor_map(p_map).
            distance_map(d_map));

    // Check that all stations have been reached
    int nvalid = 0;
    for (std::vector <Station>::iterator itr = stationList.begin();
            itr != stationList.end (); itr++)
        if (distances [(*itr).nodeIndex] < FLOAT_MAX)
            nvalid++;
    assert (nvalid == stationList.size ());

    // Trace back from each station 
    Vertex v0, v;
    dists.resize (0);
    float dist;

    for (std::vector <Station>::iterator itr = stationList.begin();
            itr != stationList.end (); itr++)
    {
        v0 = itr->nodeIndex;
        v = v0;
        dist = 0.0;
        for (Vertex u = p_map[v]; u != v; v = u, u = p_map[v]) 
        {
            std::pair<Graph_t::edge_descriptor, bool> edgePair = 
                boost::edge(u, v, gFull);
            Graph_t::edge_descriptor edge = edgePair.first;
            dist += boost::get (&bundled_edge_type::dist, gFull, edge);
        }
        dists.push_back (dist);
    }

    assert (dists.size () == stationList.size ());
    // First have to match long long fromNode to index# within stationList
    int id = -INT_MAX;
    for (int i=0; i<stationList.size(); i++)
    {
        if (stationList [i].nodeIndex == fromNode)
            id = i;
    }
    assert (id >= 0);

    for (int i=0; i<stationList.size (); i++)
        distMat (id, i) = dists [i];

    return (0);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             WRITEDMAT                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int Ways::writeDMat ()
{
    std::string fname = "distMat.csv";
    std::ofstream out_file;

    out_file.open (fname, std::ios::out);

    for (int i=0; i<distMat.size1(); i++)
    {
        for (int j=0; j<(distMat.size2() - 1); j++)
            out_file << distMat (i, j) << ", ";
        out_file << std::endl;
    }

    out_file.close ();
};

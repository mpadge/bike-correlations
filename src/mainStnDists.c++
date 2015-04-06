#include "mainStnDists.h"

int main(int argc, char *argv[]) {
    std::string city = "boston";

    Ways ways(city);
};

int Ways::readNodes ()
{
    int ipos [3];
    long long id;
    double lat, lon;
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
    int ipos;
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
                if (terminalNodeIDs.find (startNode) == terminalNodeIDs.end())
                    terminalNodeIDs.insert (startNode);
                if (terminalNodeIDs.find (endNode) == terminalNodeIDs.end())
                    terminalNodeIDs.insert (endNode);
            } // end if highway
            highway = false;
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

    std::cout << "\rRead " << terminalNodeIDs.size () <<
        " terminal nodes of ways." << std::endl;

    return terminalNodeIDs.size ();
} // end Ways::readTerminalNodes

int Ways::readWays ()
{
    bool inway = false, highway = false;
    int ipos, nnodes = 0, nways = 0;
    double dist;
    long long node; 
    uset_Itr usetitr;
    umapPair_Itr umapitr;
    std::vector <long long> waynodes, ids;
    std::vector <double> lats, lons;
    std::string linetxt, tempstr,
        fname = "/data/data/bikes/planet-boston.osm",
        fout_ways = "boston-ways.csv";
    std::ifstream in_file;
    std::ofstream out_file;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());
    in_file.clear ();
    in_file.seekg (0); 
    out_file.open (fout_ways.c_str (), std::ofstream::out);

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
                            (usetitr = terminalNodeIDs.find (*itr)) != 
                            terminalNodeIDs.end())
                    {
                        dist = calcDist (lons, lats);
                        out_file << std::to_string (ids [0]) << "," << 
                            std::to_string (*itr) << ", " <<
                            std::to_string (dist) << std::endl;
                        ids.resize (0);
                        ids.push_back (*itr);
                        lats.resize (0);
                        lons.resize (0);
                        lats.push_back (((*umapitr).second).first);
                        lons.push_back (((*umapitr).second).second);
                    }
                }
                //assert (lats.size () == waynodes.size ());
                // Then scan waynodes again, and dump all terminal node pairs
                // and intermediate distances
                // ***distance calculation goes here
                highway = false;
                nways++;
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
                nnodes++;
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
                highway = true;
        } // end else if inway
    } // end while getline
    in_file.close ();
    out_file.close();

    std::cout << "\rRead " << nways << " ways comprising " << 
        nnodes << " nodes." << std::endl;

    return nways;
} // end function readWays


double Ways::calcDist (std::vector <double> x, std::vector <double> y)
{
    double d, dsum = 0.0, x0, y0, x1 = x[0], y1 = y[0], xd, yd;
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

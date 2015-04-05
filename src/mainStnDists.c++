#include "mainStnDists.h"

int main(int argc, char *argv[]) {
    std::string city = "boston";

    Ways ways(city);
};

int Ways::readNodes ()
{
    latlons.resize (0);
    
    int ipos [3];
    long long id;
    double lat, lon;
    std::string linetxt, txt, fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());

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

            nodeMap [id] = latlons.size ();
            latlons.push_back (std::make_pair (lat, lon));
        }
    } // end while getline
    in_file.close ();

    std::cout << "Read coordinates of " << nodeMap.size () << " nodes." <<
        std::endl;

    return (nodeMap.size ());
} // end Way::readNodes


int Ways::readTerminalNodes ()
{
    bool inway = false, highway = false, inList;
    int ipos, nlines = 0, line = 0, progress [2] = {0, 1};
    long long startNode, endNode;
    std::string linetxt, fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;

    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());

    startNode = endNode = INT_MIN;

    while (getline (in_file, linetxt, '\n'))
        nlines++;
    in_file.clear ();
    in_file.seekg (0); 
    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
            inway = true;
        else if (linetxt.find ("</way>") != std::string::npos)
        {
            inway = false;
            if (highway && startNode != endNode)
            {
                if (terminalNodes.find (startNode) == terminalNodes.end())
                    terminalNodes.insert (startNode);
                if (terminalNodes.find (endNode) == terminalNodes.end())
                    terminalNodes.insert (endNode);
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
        line++;
        progress [0] = floor (1000.0 * (double) line / (double) nlines);
        if (progress [0] == progress [1])
        {
            std::cout << "\rReading terminal nodes of ways: progress = " << 
                (double) progress [0] / 10.0 <<
                "% with " << terminalNodes.size () << " terminal nodes ";
            std::cout.flush ();
            progress [1]++;
        }
    } // end while getline
    in_file.close ();

    std::cout << std::endl;

    return terminalNodes.size ();
} // end Ways::readTerminalNodes

int Ways::readWays ()
{
    bool inway = false, highway = false;
    int ipos, nnodes = 0, nways = 0, nlines = 0, line = 0, 
        progress [2] = {0, 1};
    umap_Itr uitr;
    std::vector <long long> waynodes;
    std::vector <double> lats, lons;
    std::string linetxt, fname = "/data/data/bikes/planet-boston.osm";
    std::ifstream in_file;
    
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());

    while (getline (in_file, linetxt, '\n'))
        nlines++;
    in_file.clear ();
    in_file.seekg (0); 
    while (getline (in_file, linetxt, '\n'))
    {
        if (linetxt.find ("<way") != std::string::npos)
        {
            inway = true;
            waynodes.resize (0);
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
                    if ((uitr = nodeMap.find (*itr)) != nodeMap.end())
                    {
                        ipos = (*uitr).second;
                        lats.push_back (latlons [ipos].first);
                        lons.push_back (latlons [ipos].second);
                    }
                }
                // ***distance calculation goes here
                assert (lats.size () == waynodes.size ());
                highway = false;
                nways++;
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
                waynodes.push_back (atoll (linetxt.c_str ()));
                nnodes++;
            }
            else if (linetxt.find ("k=\"highway\"") != std::string::npos)
                highway = true;
        } // end else if inway
        line++;
        progress [0] = floor (1000.0 * (double) line / (double) nlines);
        if (progress [0] == progress [1])
        {
            std::cout << "\rReading ways: progress = " << 
                (double) progress [0] / 10.0 << "% ";
            std::cout.flush ();
            progress [1]++;
        }
    } // end while getline
    in_file.close ();
    std::cout << std::endl << "There are " << nways << " ways comprising " << 
        nnodes << " nodes." << std::endl;

    return 0;
} // end function readWays

#include "StationData.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           GETDIRNAME                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

std::string Stations::GetDirName ()
{
    std::ifstream in_file;
    std::string dirtxt;
    std::string configfile = "bikes.cfg"; // Contains name of data directory
    struct dirent *ent;

    in_file.open (configfile.c_str (), std::ifstream::in);
    assert (!in_file.fail ());

    while (!in_file.eof ())
    {
        getline (in_file, dirtxt, '\n');
        if (dirtxt.find (':') < std::string::npos) 
        {
            dirtxt = dirtxt.substr (0, dirtxt.find (':'));
            std::transform (dirtxt.begin(), dirtxt.end(), 
                    dirtxt.begin(), ::tolower);
            if (dirtxt.substr (0, 3) == _city.substr (0, 3)) 
            {
                getline (in_file, dirtxt, '\n');
                break;
            }
        }
    }
    in_file.close ();
    return dirtxt;
} // end Stations::GetDirName

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           GETDIRLIST                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void StationData::GetDirList ()
{
    std::string fname;
    DIR *dir;
    struct dirent *ent;

    filelist.resize (0);
    if ((dir = opendir (_dirName.c_str())) != NULL) 
    {
        while ((ent = readdir (dir)) != NULL) 
        {
            fname = ent->d_name;
            if (fname != "." && fname != "..") 
            {
                fname = fname;
                filelist.push_back (fname);
            }
        }
        closedir (dir);
        std::sort (filelist.begin(), filelist.end());
    } else {
        std::string outstr = "ERROR: Directory for city = " +\
                              _city + " at " + _dirName + " does not exist";
        perror ("");
        std::cout << outstr << std::endl;
        //return EXIT_FAILURE;
    }
} // end StationData::GetDirList


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          GETSTATIONS                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int StationData::GetStations ()
{
    /*
     * Reads from station_latlons which is constructed with getLatLons.py and
     * *MUST* be ordered numerically. Returns _numStations
     *
     * TODO: Write handlers for cases where there are trips to/from stations not
     * in StationList.
     * TODO: Update StationList for nyc, because there seem to be trips to/from
     * one station that is not in list.
     * 
     */
    const std::string dir = "data/"; 
    int ipos, tempi, count;
    bool tube;
    OneStation oneStation;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt;

    StationList.resize (0);
    count = 0;
    oneStation.name = "";

    if (_city == "london" || _city == "nyc")
    {
        fname = dir + "station_latlons_" + _city + ".txt";
        in_file.open (fname.c_str (), std::ifstream::in);
        assert (!in_file.fail ());
        in_file.clear ();
        in_file.seekg (0); 
        getline (in_file, linetxt, '\n'); // header
        while (!in_file.eof ()) 
        {
            getline (in_file, linetxt,'\n');
            if (linetxt.length () > 1) 
            {
                ipos = linetxt.find(',',0);
                tempi = atoi (linetxt.substr (0, ipos).c_str());
                if (tempi > count) 
                    count = tempi;
                oneStation.ID = tempi;
                linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
                ipos = linetxt.find (',', 0);
                oneStation.lat = atof (linetxt.substr (0, ipos).c_str());
                linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
                ipos = linetxt.find (',', 0);
                oneStation.lon = atof (linetxt.substr (0, ipos).c_str());
                StationList.push_back (oneStation);
            }
        }
        in_file.close();
    } else if (_city == "oysterRail" || _city == "oysterTube")
    {
        oneStation.ID = INT_MIN;
        if (_city == "oysterRail")
        {
            tube = false;
            fname = dir + "London-rail-stations.txt";
        }
        else
        {
            tube = true;
            fname = dir + "London-tube-stations.txt";
        }
        in_file.open (fname.c_str (), std::ifstream::in);
        assert (!in_file.fail ());
        in_file.clear ();
        in_file.seekg (0); 
        getline (in_file, linetxt,'\n');
        while (!in_file.eof ()) 
        {
            getline (in_file, linetxt,'\n');
            if (linetxt.length () > 1) 
            {
                ipos = linetxt.find(',',0);
                oneStation.name = standardise (linetxt.substr (0, ipos).c_str());
                oneStation.name = substituteNames (tube, oneStation.name);
                linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
                ipos = linetxt.find (',', 0);
                oneStation.lat = atof (linetxt.substr (0, ipos).c_str());
                linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
                oneStation.lon = atof (linetxt.c_str());
                StationList.push_back (oneStation);
            }
        }
        in_file.close();
    }

    return count;
} // end StationData::GetStations


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          MAKESTATIONINDEX                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void StationData::MakeStationIndex ()
{
    // First station is #1 and last is _maxStation, so _StationIndex has 
    // len (_maxStns + 1), with _StationIndex [sti.ID=1] = 0 and
    // _StationIndex [sti.ID=_maxStation] = _numStations.
    OneStation sti;

    _StationIndex.resize (_maxStation + 1);
    for (std::vector <int>::iterator pos=_StationIndex.begin();
            pos != _StationIndex.end(); pos++)
        *pos = INT_MIN;
    for (int i=0; i<StationList.size (); i++) 
    {
        sti = StationList [i];
        _StationIndex [sti.ID] = i;
    }
} // end StationData::MakeStationIndex


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             COUNTTRIPS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

double StationData::CountTrips ()
{
    double count = 0.0;

    for (int i=0; i<ntrips.size1(); i++)
        for (int j=0; j<ntrips.size2(); j++)
            count += ntrips (i, j);

    return (count);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                              READDMAT                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int StationData::readDMat ()
{
    std::string distFile;
    if (_city == "london")
        distFile = "data/station_dists_london.txt";
    else if (_city == "nyc")
        distFile = "data/station_dists_nyc.txt";
    else if (_city == "oysterTube")
        distFile = "data/London-tube-station-dists.txt";
    else if (_city == "oysterRail")
        distFile = "data/London-rail-station-dists.txt";

    int count, ipos, tempi [2];
    double d;
    std::ifstream in_file;
    std::string linetxt;
    in_file.open (distFile.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // TODO: INSERT ERROR HANDLER
        return -1;
    } 
    in_file.clear ();
    in_file.seekg (0); 
    count = 0;
    
    if (_city == "london" || _city == "nyc")
    {
        while (getline (in_file, linetxt, '\n')) {
            ipos = linetxt.find(',',0);
            tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find(',',0);
            tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
            tempi [0] = _StationIndex [tempi[0]];
            tempi [1] = _StationIndex [tempi[1]];
            if (tempi[0] < 0 | tempi[0] > returnNumStations ())
                std::cout << "ERROR: stn[0]#" << tempi[0] << std::endl;
            if (tempi[1] < 0 | tempi[1] > returnNumStations ())
                std::cout << "ERROR: stn[1]#" << tempi[1] << std::endl;
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            d = atof (linetxt.c_str());
            dists (tempi [0], tempi [1]) = dists (tempi [1], tempi [0]) = d;
            count++;
        }
    }
    else // Distances between train stations, read direct from matrix
    {
        while (getline (in_file, linetxt, '\n'))
            count++;
        if (count != _numStations)
        {
            std::cout << "ERROR: " << distFile << " does not have " <<
                _numStations << " columns and rows!" << std::endl;
            return -1;
        }
        else
        {
            in_file.clear ();
            in_file.seekg (0); 
            for (int i=0; i<_numStations; i++)
            {
                getline (in_file, linetxt, '\n');
                for (int j=0; j<(_numStations - 1); j++)
                {
                    ipos = linetxt.find (',',0);
                    dists (i, j) = dists (j, i) = 
                        atof (linetxt.substr (0, ipos).c_str());
                    linetxt = linetxt.substr (ipos + 1, 
                            linetxt.length () - ipos - 1);
                } // end for j over columns
                dists (i, _numStations - 1) = dists (_numStations - 1, i) =
                    atof (linetxt.c_str ());
            } // end for i over rows
        } // end else count == _numStations
    }

    in_file.close ();

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             WRITEDMAT                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int StationData::writeDMat ()
{
    int numStations = StationList.size ();
    std::string distFile, nameFile;
    std::ofstream dists_out, names_out;

    if (_city == "london")
        distFile = "stationDistsMat_london.csv";
    else if (_city == "nyc")
        distFile = "stationDistsMat_nyc.csv";
    else if (_city == "oysterTube")
    {
        distFile = "DistMatTube.csv";
        nameFile = "DistMatTube_StationList.csv";
    }
    else if (_city == "oysterRail")
    {
        distFile = "DistMatRail.csv";
        nameFile = "DistMatRail_StationList.csv";
    }
    
    dists_out.open (distFile.c_str (), std::ofstream::out);
    names_out.open (nameFile.c_str (), std::ofstream::out);

    for (int i=0; i<numStations; i++)
    {
        names_out << StationList[i].name << ", " << StationList [i].lat <<
            ", " << StationList [i].lon << std::endl;
        for (int j=0; j<(numStations - 1); j++)
            dists_out << dists (i, j) << ", ";
        dists_out << dists (i, numStations - 1) << std::endl;
    }
    dists_out.close ();
    names_out.close ();

    std::cout << numStations << " inter-station distances written to " << 
        distFile.c_str () << std::endl;
    if (_city.substr (0, 6) == "oyster")
        std::cout << "\t... and corresponding station names written to " <<
            nameFile.c_str () << std::endl;

    return 0;
}

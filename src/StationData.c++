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
            fname = dir + "London-rail-stations.txt";
        else
            fname = dir + "London-tube-stations.txt";
        in_file.open (fname.c_str (), std::ifstream::in);
        assert (!in_file.fail ());
        in_file.clear ();
        in_file.seekg (0); 
        while (!in_file.eof ()) 
        {
            getline (in_file, linetxt,'\n');
            if (linetxt.length () > 1) 
            {
                ipos = linetxt.find(',',0);
                oneStation.name = standardise (linetxt.substr (0, ipos).c_str());
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

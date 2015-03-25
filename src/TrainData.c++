#include "TrainData.h"


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           GETTRAINDATA                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::getTrainData (bool tube)
{
    /*
     * There are 376 rail and 608 tube stations, and yet only 90 and 302 of
     * these respectively appear in the oystercard data.  These 90 or 302
     * station names are aligned here with the (often different) names of the
     * tube and NR stations, and _StationIndex is filled with 90 or 302 entries
     * correponding to the indices into the 376 or 608 of the latter stations.
     * These include lat-lons, and so enable station coordinates to easily be
     * obtained.
     *
     * This indexing is initially done by reading the osytercard data only to
     * extract the station names and dump them to a file called
     * "oystercardnames.csv". If this file exists, it is simply read instead.
     */
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len, sum = 0;
    std::ifstream in_file; 
    std::ofstream out_file;

    int ID, count = 0, ipos, tempi [2];
    std::string modeTxt, mode, start, stop, linetxt;
    bool startIn, stopIn;

    if (tube)
        modeTxt = "tube";
    else
        modeTxt = "rail";

    // First construct an index of oystercardnames into StationList.name. This
    // Read oystercardnames.csv if it exists, otherwise read raw data to extract
    // a vector of unique names and associated modes (NR/LUL).
    struct oysterOne
    {
        int index; // into StationData.StationIndex
        std::string mode, name;
    };
    std::vector <oysterOne> _OysterStations;
    _OysterStations.resize (0);
    std::string dirName = "/data/data/", 
        fname_base = "/data/data/oystercardjourneyinformation.zip",
        fname_oyster = "./data/oystercardnames" + modeTxt + ".csv";

    in_file.open (fname_oyster.c_str());
    if (in_file)
    {
        in_file.clear ();
        in_file.seekg (0); // Both lines needed to rewind file.
        tempi [0] = tempi [1] = 0;
        while (getline (in_file, linetxt, '\n'))
        {
            ipos = linetxt.find(",",0);
            mode = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            _OysterStations.push_back ({INT_MIN, mode, linetxt});
            count++;
        }
        in_file.close();
        std::cout << "Read " << count << " " << modeTxt << 
            " stations from " << fname_oyster << std::endl;
    }
    else
    {
        std::cout << "No oystercardnames" << modeTxt << 
            ".csv; reading them from raw data ..." << std::endl;

        std::string fname_csv;
        archive = fname_base.c_str ();
        if ((za = zip_open(archive, 0, &err)) == NULL) {
            zip_error_to_str(buf, sizeof(buf), err, errno);
            std::cout << stderr << archive << "can't open size archive : " <<
                buf << std::endl;
            return -1;
        } 
        int nfiles = zip_get_num_entries (za, 0);
        for (int i=0; i<nfiles; i++)
        {
            if (zip_stat_index (za, i, 0, &sb) == 0) {
                fname_csv = dirName + sb.name;
                zf = zip_fopen_index(za, 0, 0);
                if (!zf) {
                    std::cout << stderr << "ERROR: cannot open file#" <<
                        i << " in archive " << archive << std::endl;
                    return 1;
                }
                out_file.open (fname_csv.c_str(), std::ios::out);
                while (sum != sb.size) {
                    len = zip_fread(zf, buf, 100);
                    if (len < 0) {
                        // INSERT ERROR HANDLER
                    }
                    out_file.write (buf, len);
                    sum += len;
                }
                out_file.close ();

                zip_fclose(zf); 
            }
        }
        zip_close (za);

        in_file.open (fname_csv.c_str (), std::ifstream::in);
        if (in_file.fail ()) {
            // INSERT ERROR HANDLER
            return -1;
        }
        in_file.clear ();
        in_file.seekg (0); // Both lines needed to rewind file.
        getline (in_file, linetxt, '\n');

        while (getline (in_file, linetxt, '\n')) { 
            ipos = linetxt.find("\",\"",0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            mode = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            start = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            stop = linetxt.substr (0, ipos);

            if ((mode == "NR" || mode == "LUL" || mode == "LUL/DLR" ||
                        mode == "DLR") && (start != "Unstarted" &&
                    stop != "Unfinished" && start != "Bus" && stop != "Bus" &&
                    start != "Not Applicable" && stop != "Not Applicable"))
            {
                startIn = stopIn = false;
                for (std::vector <oysterOne>::iterator 
                        pos =_OysterStations.begin();
                        pos != _OysterStations.end(); pos++)
                {
                    if ((*pos).name == start)
                        startIn = true;
                    if ((*pos).name == stop) 
                        stopIn = true;
                }
                if ((tube && mode != "NR") || (!tube && mode == "NR"))
                {
                    if (!startIn)
                        _OysterStations.push_back ({INT_MIN, mode, start});
                    if (!stopIn && startIn)
                        _OysterStations.push_back ({INT_MIN, mode, stop});
                    count++; 
                    // Use these lines to examine whether stations not in list are NR or
                    // LUL:
                    /*
                    if (count < 1000 && (start == "Harrow Wealdstone" || 
                                stop == "Harrow Wealdstone"))
                        std::cout << "******" << mode << ": " << start << 
                            "->" << stop << "***" << std::endl;
                    */
                }
            }
        } // end while getline
        in_file.close();
        remove (fname_csv.c_str ());
        std::sort (_OysterStations.begin(), _OysterStations.end(),
                [] (oysterOne a, oysterOne b){ return a.name < b.name; });

        std::cout << "The oystercard data has " << count << " trips between " <<
            _OysterStations.size () << 
            " stations; station names writte to " << 
            fname_oyster << std::endl;

        out_file.open (fname_oyster.c_str());
        for (std::vector <oysterOne>::iterator pos=_OysterStations.begin();
                pos != _OysterStations.end(); pos++)
            out_file << (*pos).mode << "," << (*pos).name << std::endl;
        out_file.close ();
    } // end else no oystercardnames.csv
    // Then standardise the way station names are written
    for (std::vector <oysterOne>::iterator pos=_OysterStations.begin();
            pos != _OysterStations.end(); pos++)
        (*pos).name = standardise ((*pos).name);

    /*
     * Then match the OysterStations.names to names of the tube or NR
     * stations, and fill the OysterStations.index into the latter.
     */
    std::cout << "***lengths of oysterstations and stationlist = (" <<
        _OysterStations.size() << ", " << StationList.size () <<
        ")" << std::endl;

    std::string stName, stni;
    bool check;
    count = 0;
    for (std::vector <oysterOne>::iterator pos=_OysterStations.begin();
            pos != _OysterStations.end(); pos++)
    {
        if ((tube && (*pos).mode == "LUL") || (!tube && (*pos).mode == "NR"))
        {
            check = false;
            stName = standardise ((*pos).name);
            for (std::vector <OneStation>::iterator stn=StationList.begin();
                    stn != StationList.end(); stn++)
            {
                if ((*stn).name == stName)
                    check = true;
                if (stName == "queens park")
                {
                    //std::cout << "***" << stName << "***" << 
                    //    (*stn).name << "***" << (*pos).name << "***" <<
                    //    (*pos).mode << "***" << std::endl;
                }
            }
            if (!check)
                std::cout << "Station <" << count << "/" << _OysterStations.size () <<
                    ": " << stName << "> not in StationList" << std::endl;
            else
                count++;
        }
    }
    /*
    for (std::vector <OneStation>::iterator stn=StationList.begin();
            stn != StationList.end(); stn++)
            //std::cout << "SL:" << (*stn).name << std::endl;
    */

    std::cout << "***count = " << count << "; oysterstaitons.size = " <<
        _OysterStations.size () << std::endl;
    if (count != _OysterStations.size ())
        std::cout << "ERROR: Not all stations in StationList could be " <<
            " indexed into Rail Station List." << std::endl;
    // TODO: Insert proper error handler

    return 0;
} // end TrainData::getTrainData


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETTRAINSTATIONS                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::getTrainStations ()
{
    /*
     * There are 976 tube and NR stations, and yet only 392 of these appear in
     * the oystercard data. To avoid making big ntrips matrices (and all
     * others), only to have to reduce them later, the relevant stations are
     * first extracted here by reading all oystercard data.
     *
     * These station names are dumped to a file called "oystercardnames.csv". If
     * this file exists, then it is subsequently read instead of loading all raw
     * data again. Thus, if new raw data appear, the file can simply be deleted
     * and will be automatically regenerated the next time.
     *
     * These 392 station names are then aligned here with the (often different)
     * names of the tube and NR stations, and _StationIndex is filled with 392
     * entries correponding to the indices into the 976 of the latter stations.
     * These include lat-lons, and so enable station coordinates to easily be
     * obtained.
     */
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len, sum = 0;
    std::ifstream in_file; 
    std::ofstream out_file;

    int ID, count = 0, ipos, tempi [2];
    std::string mode, start, stop, linetxt;
    bool startIn, stopIn;

    // First check if oystercardnames.csv exists
    struct oysterOne
    {
        std::string mode, name;
    };
    std::vector <oysterOne> _OysterStations;
    _OysterStations.resize (0);
    std::string dirName = "/data/data/", 
        fname_oyster = "./data/oystercardnames.csv", 
        fname_base = "/data/data/oystercardjourneyinformation.zip";
    in_file.open (fname_oyster.c_str());
    if (in_file)
    {
        in_file.clear ();
        in_file.seekg (0); // Both lines needed to rewind file.
        tempi [0] = tempi [1] = 0;
        while (getline (in_file, linetxt, '\n'))
        {
            ipos = linetxt.find(",",0);
            mode = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            _OysterStations.push_back ({mode, linetxt});
            if (mode == "NR")
                tempi [0]++;
            else
                tempi [1]++;

        }
        in_file.close();
        std::cout << "There are " << tempi [0] << " NR + " << tempi [1] <<
            " LUL = " << _OysterStations.size () << 
            " stations in the oystercard data." << std::endl;
    }
    else
    {
        std::cout << "No oystercardnames.csv; reading them from raw data ..." <<
            std::endl;

        std::string fname_csv;
        archive = fname_base.c_str ();
        if ((za = zip_open(archive, 0, &err)) == NULL) {
            zip_error_to_str(buf, sizeof(buf), err, errno);
            std::cout << stderr << archive << "can't open size archive : " <<
                buf << std::endl;
            return -1;
        } 
        int nfiles = zip_get_num_entries (za, 0);
        for (int i=0; i<nfiles; i++)
        {
            if (zip_stat_index (za, i, 0, &sb) == 0) {
                fname_csv = dirName + sb.name;
                zf = zip_fopen_index(za, 0, 0);
                if (!zf) {
                    std::cout << stderr << "ERROR: cannot open file#" <<
                        i << " in archive " << archive << std::endl;
                    return 1;
                }
                out_file.open (fname_csv.c_str(), std::ios::out);
                while (sum != sb.size) {
                    len = zip_fread(zf, buf, 100);
                    if (len < 0) {
                        // INSERT ERROR HANDLER
                    }
                    out_file.write (buf, len);
                    sum += len;
                }
                out_file.close ();

                zip_fclose(zf); 
            }
        }
        zip_close (za);

        in_file.open (fname_csv.c_str (), std::ifstream::in);
        if (in_file.fail ()) {
            // INSERT ERROR HANDLER
            return -1;
        }
        in_file.clear ();
        in_file.seekg (0); // Both lines needed to rewind file.
        getline (in_file, linetxt, '\n');

        while (getline (in_file, linetxt, '\n')) { 
            ipos = linetxt.find("\",\"",0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            mode = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            start = linetxt.substr (0, ipos);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            stop = linetxt.substr (0, ipos);

            if ((mode == "NR" || mode == "LUL" || mode == "LUL/DLR" ||
                        mode == "DLR") && (start != "Unstarted" &&
                    stop != "Unfinished" && start != "Bus" && stop != "Bus" &&
                    start != "Not Applicable" && stop != "Not Applicable"))
            {
                startIn = stopIn = false;
                for (std::vector <oysterOne>::iterator 
                        pos =_OysterStations.begin();
                        pos != _OysterStations.end(); pos++)
                {
                    if ((*pos).name == start)
                        startIn = true;
                    if ((*pos).name == stop) 
                        stopIn = true;
                }
                if (!startIn)
                    _OysterStations.push_back ({mode, start});
                if (!stopIn && startIn)
                    _OysterStations.push_back ({mode, stop});
                count++; 
                // Use these lines to examine whether stations not in list are NR or
                // LUL:
                /*
                if (count < 1000 && (start == "Harrow Wealdstone" || 
                            stop == "Harrow Wealdstone"))
                    std::cout << "******" << mode << ": " << start << 
                        "->" << stop << "***" << std::endl;
                */
            }
        } // end while getline
        in_file.close();
        remove (fname_csv.c_str ());
        std::sort (_OysterStations.begin(), _OysterStations.end(),
                [] (oysterOne a, oysterOne b){ return a.name < b.name; });

        std::cout << "The oystercard data has " << count << " trips between " <<
            _OysterStations.size () << 
            " stations; station names writte to " << 
            fname_oyster << std::endl;

        out_file.open (fname_oyster.c_str());
        for (std::vector <oysterOne>::iterator pos=_OysterStations.begin();
                pos != _OysterStations.end(); pos++)
            out_file << (*pos).mode << "," << (*pos).name << std::endl;
        out_file.close ();
    } // end else no oystercardnames.csv

    /*
     * Then match the OysterStationNames to names of the 976 tube and NR
     * stations, and create a corresponding index into the latter.
     *
     * The 976 stations in RailStationList are read at construction from the two
     * tube and NR files.  The OysterStationNames are sometimes written
     * differently in the trip data to how they appear in the "official"
     * RailStationList, so the following list of possible substitutions is
     * scanned to find potential matches with alternative versions.
     */
    bool tube;
    size_t found;
    std::string stName;
    struct subs
    {
        bool tube;
        std::string told, tnew;
    };
    std::vector <subs> strSubs;
    strSubs.push_back ({true, "Street","St"});
    strSubs.push_back ({false, "Street","St"});
    strSubs.push_back ({true, "Road","Rd"});
    strSubs.push_back ({false, "Road","Rd"});
    strSubs.push_back ({true, "(District)", "D"});
    strSubs.push_back ({true, "(Met.)", "M"});
    strSubs.push_back ({true, "(Bakerloo)", "B"});
    strSubs.push_back ({true, "Park", "Pk"});
    strSubs.push_back ({true, "Terminal", "Term"}); // Heathrow
    strSubs.push_back ({true, "Terminals 1 2 3", "Terms 123"}); 
    strSubs.push_back ({true, "Kensington", "Kens"});
    strSubs.push_back ({true, "Bromley-by-Bow", "Bromley By Bow"});
    strSubs.push_back ({true, "Road and Barnsbury", "Rd&B'sby"});

    // From this point on modes of rail (tube=t/f) have been determined by
    // examining the modes associated with the actual trips corresponding to
    // these station names using the commented-out lines above
    strSubs.push_back ({true, "Balham", "Balham SCL"});
    strSubs.push_back ({false, "Fenchurch Street", "Fenchurch St NR"});
    strSubs.push_back ({false, "Fenchurch Street", "FENCHURCH ST NR"});
    strSubs.push_back ({true, "Canary Wharf", "Canary Wharf E2"});
    strSubs.push_back ({true, "Crossharbour and London Arena", "Crossharbour"});
    strSubs.push_back ({true, "Cutty Sark for Maritime Greenwich", "Cutty Sark"});
    strSubs.push_back ({true, 
            "Edgware Road (Circle/District/Hammersmith and City)",
            "Edgware Road M"}); // just a presumption there
    strSubs.push_back ({true, "Harrow-on-the-Hill", "Harrow On The Hill"});
    strSubs.push_back ({true, "Harrow and Wealdstone", "Harrow Wealdstone"});
    strSubs.push_back ({false, "Harrow & Wealdstone", "Harrow Wealdstone"});
    strSubs.push_back ({true, "Highbury and Islington", "Highbury"});
    strSubs.push_back ({false, "Kensington (Olympia)", "Kensington Olympia"});
    strSubs.push_back ({true, "King's", "Kings"});
    strSubs.push_back ({false, "King's", "Kings"});
    strSubs.push_back ({true, "King's Cross", "Kings Cross M"});
    strSubs.push_back ({true, "King's Cross", "Kings Cross T"});
    strSubs.push_back ({false, "Liverpool Street", 
            "Liverpool St WAGN TOC Gates"});
    strSubs.push_back ({false, "Norwood Junction", "Norwood Junction SR"});
    strSubs.push_back ({false, "Paddington", "Paddington FGW"});
    strSubs.push_back ({false, "Rainham", "Rainham Essex"});
    strSubs.push_back ({true, "Shepherds Bush Market", "Shepherd's Bush Mkt"});
    strSubs.push_back ({true, "Shepherds Bush Market", "Shepherd's Bush Und"});
    strSubs.push_back ({true, "St. James's Park", "St James's Park"});
    strSubs.push_back ({true, "St. Johns Wood", "St Johns Wood"});
    strSubs.push_back ({false, "St Pancras", "St Pancras International"});
    strSubs.push_back ({true, "St. Pauls", "St Pauls"});
    strSubs.push_back ({false, "Sudbury & Harrow Road", "Sudbury&Harrow Rd"});
    strSubs.push_back ({false, "Sutton", "Sutton Surrey"});
    strSubs.push_back ({false, "Sydenham", "Sydenham SR"});
    strSubs.push_back ({true, "Totteridge and Whetstone", "Totteridge"});
    strSubs.push_back ({false, "Victoria", "Victoria TOCs"});
    strSubs.push_back ({true, "Waterloo", "Waterloo JLE"});
    strSubs.push_back ({true, "Watford", "Watford Met"});
    strSubs.push_back ({false, "West Hampstead", "West Hampst'd NL"});

    std::string strAlt;

    /*
     * The following lines fill the int _stationIndex [392] vector with indices
     * into RailStationList[976].
     */


    count = 0;
    _StationIndex.resize (0);
    for (std::vector <oysterOne>::iterator pos=_OysterStations.begin();
            pos != _OysterStations.end(); pos++)
    {
        startIn = false;
        for (int i=0; i<StationList.size (); i++) // Was RailStationList
        {
            stName = StationList [i].name;
            if ((*pos).name == stName || (*pos).name == (stName + " NR"))
            {
                startIn = true;
                _StationIndex.push_back (i);
                break;
            }
            else // search alternatives
            {
                for (std::vector <subs>::iterator poss=strSubs.begin();
                        poss != strSubs.end(); poss++)
                {
                    strAlt = stName;
                    if (tube == (*poss).tube &&
                            (found = strAlt.find ((*poss).told)) != 
                            std::string::npos)
                    {
                        strAlt.replace (found, (*poss).told.length(), (*poss).tnew);
                        if ((*pos).name == strAlt || 
                                (!tube && (*pos).name == (strAlt + " NR")))
                        {
                            startIn = true;
                            _StationIndex.push_back (i);
                            break;
                        }
                    }
                } // end for iterator over strSubs
                if (startIn)
                    break;
            }
        } // end for i over RailStationList
        for (int i=0; i<StationList.size (); i++) // Was TubeStationList
        {
            stName = StationList [i].name;
            if ((*pos).name == stName || (*pos).name == (stName + " DLR"))
            {
                startIn = true;
                _StationIndex.push_back (i);
                break;
            }
            else // search alternatives
            {
                for (std::vector <subs>::iterator poss=strSubs.begin();
                        poss != strSubs.end(); poss++)
                {
                    strAlt = stName;
                    if (tube == (*poss).tube &&
                            (found = strAlt.find ((*poss).told)) != 
                            std::string::npos)
                    {
                        strAlt.replace (found, (*poss).told.length(), (*poss).tnew);
                        if ((*pos).name == strAlt || 
                                (!tube && (*pos).name == (strAlt + " NR")))
                        {
                            startIn = true;
                            _StationIndex.push_back (i);
                            break;
                        }
                    }
                } // end for iterator over strSubs
                if (startIn)
                    break;
            }
        } // end for i over RailStationList
        if (!startIn)
        {
            std::cout << "---" << count << "-" << (*pos).name << 
                " not in Rail or Tube Station List" << std::endl;
            count++;
        }
    }

    strSubs.resize (0);

    if (_StationIndex.size () != _OysterStations.size ())
        std::cout << "ERROR: Not all stations in StationList could be " <<
            " indexed into Rail Station List." << std::endl;
    // TODO: Insert proper error handler

    return _StationIndex.size ();
}

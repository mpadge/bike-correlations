#include "TrainData.h"




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
        for (int i=0; i<RailStationList.size (); i++)
        {
            stName = RailStationList [i];
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
        for (int i=0; i<TubeStationList.size (); i++)
        {
            stName = TubeStationList [i];
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

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          GETTRAINTRIPS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::getTrainTrips ()
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
    _OysterStations.resize (0);
    std::string dirName = "/data/data/", 
        fname_oyster = "./data/oystercardnames.csv", 
        fname_base = "/data/data/oystercardjourneyinformation.zip";

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

    return 0;
}



/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           WRITENUMTRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::writeNumTrips ()
{
    int numStations = TrainData::getNumTubeStations ();
    std::string fname = "NumTrips_Oyster.csv";
    // TODO: Write separate Rail and Tube trips

    std::ofstream out_file;
    out_file.open (fname.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++)
    {
        for (int j=0; j<numStations; j++)
        {
            out_file << ntrips (i, j);
            if (j == (numStations - 1))
                out_file << std::endl;
            else
                out_file << ", ";
        }
    }
    out_file.close ();
    std::cout << "Numbers of trips written to " << fname.c_str () << std::endl;

    return 0;
}

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                              CALCR2                                **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

// Also calculates covariance

int TrainData::calcR2 (bool from)
{
    bool standardise = TrainRideData::getStandardise();
    int tempi, numStations = TrainRideData::getNumTubeStations ();
    // TODO: Repeat for Rail and Tube stations
    double tempd [2];
    std::vector <double> x0, y0, x1, x2, y2, d;
    RegrResults regrResults;

    d.resize (0);

    /* Double loop over numStations requires multiple (x,y) vectors
     * x0 is held in outer loop for i over (numStations - 1), potentially in
     * standardised (0-1) form.
     * y0 is held in inner loop for j over (i+1):numStations, also potentially
     * in standardised (0-1) form.
     *
     * For each of these inner loops, both vectors may be modified through
     * reduction to near/far stations only and/or removal of zeros. In each
     * case/both cases, the modified vectors are stored as (x2, y2), and then
     * copied after modification to (x1, y0), from which the correlations are
     * evaluated.
     *
     * x1 is necessary to allow it to revert to x0 in each inner loop.
     * The code ensures that all vectors have the same lengths at all times, and
     * so explicit loops are used for clarity rather than iterators.
     */
    for (int i=0; i<(numStations-1); i++)
    {
        x0.resize (0);
        for (int j=0; j<numStations; j++)
            if (from)
                x0.push_back (ntrips (i, j));
            else
                x0.push_back (ntrips (j, i));
        if (standardise)
        {
            tempd [0] = 0.0;
            for (int j=0; j<numStations; j++)
                tempd [0] += x0 [j];
            for (int j=0; j<numStations; j++)
                x0 [j] = x0 [j] / tempd [0];
        }
        for (int j=(i+1); j<numStations; j++)
        {
            x1.resize (0);
            for (int k=0; k<numStations; k++)
                x1.push_back (x0 [k]);

            y0.resize (0);
            for (int k=0; k<numStations; k++)
                if (from)
                    y0.push_back (ntrips (j, k));
                else
                    y0.push_back (ntrips (k, j));

            if (standardise)
            {
                tempd [0] = tempd [1] = 0.0;
                for (int k=0; k<numStations; k++)
                    tempd [0] += y0 [k];
                for (int k=0; k<numStations; k++)
                    y0 [k] = y0 [k] / tempd [0];
            }

            if (nearfar != 0) // Remove half of stations from lists
            {
                d.resize (0);
                for (int k=0; k<numStations; k++)
                    d.push_back (dists (i, k) + dists (j, k));
                std::sort (d.begin(), d.end());
                tempi = floor (d.size () / 2);
                tempd [0] = (d [tempi] + d [tempi + 1]) / 2.0;
                d.resize (0);
                for (int k=0; k<numStations; k++)
                    d.push_back (dists (i, k) + dists (j, k));
                x2.resize (0);
                y2.resize (0);
                for (int k=0; k<numStations; k++)
                {
                    if (nearfar == 1 && d[k] < tempd [0])
                    {
                        x2.push_back (x1[k]);
                        y2.push_back (y0[k]);
                    } else if (nearfar == 2 && d[k] > tempd [0])
                    {
                        x2.push_back (x1[k]);
                        y2.push_back (y0[k]);
                    }
                }
                x1.resize (0);
                y0.resize (0);
                for (int k=0; k<x2.size(); k++)
                {
                    x1.push_back (x2 [k]);
                    y0.push_back (y2 [k]);
                }
                x2.resize (0);
                y2.resize (0);
                d.resize (0);
            } // end if nearfar
            if (ignoreZeros)
            {
                x2.resize (0);
                y2.resize (0);
                for (int k=0; k<x1.size(); k++)
                    if (x1 [k] > 0.0 && y0 [k] > 0.0)
                    {
                        x2.push_back (x1 [k]);
                        y2.push_back (y0 [k]);
                    }
                x1.resize (0);
                y0.resize (0);
                for (int k=0; k<x2.size(); k++)
                {
                    x1.push_back (x2 [k]);
                    y0.push_back (y2 [k]);
                }
                x2.resize (0);
                y2.resize (0);
            } // end if ignoreZeros
            regrResults = regression (x1, y0);
            r2 (i, j) = r2 (j, i) = regrResults.r2;
            cov (i, j) = cov (j, i) = regrResults.cov;
        } // end for j over (i+1):numStations
    } // end for i over (numStations - 1)
    x0.resize (0);
    x1.resize (0);
    y0.resize (0);

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             WRITER2MAT                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::writeR2Mat (bool from)
{
    int numStations = TrainRideData::getNumTubeStations ();
    // TODO: Repeat for Rail and Tube stations
    std::string r2File;
    if (from)
        r2File = "R2_oyster_from.csv";
    else
        r2File = "R2_oyster_to.csv";

    std::ofstream out_file;
    out_file.open (r2File.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++) {
        for (int j=0; j<numStations; j++) {
            out_file << r2 (i, j);
            if (j == (numStations - 1))
                out_file << std::endl;
            else
                out_file << ", ";
        }
    }
    out_file.close ();
    std::cout << "Correlations written to " << r2File.c_str () << std::endl;

    return 0;
}

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            WRITECOVMAT                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::writeCovMat (bool from)
{
    int numStations = TrainRideData::getNumTubeStations ();
    // TODO: Repeat for Rail and Tube stations
    std::string covFile, stdext;
    bool standardise = TrainRideData::getStandardise ();
    if (standardise)
        stdext = "_std";
    else
        stdext = "_unstd";

    if (from)
        r2File = "Cov_oyster_from.csv";
    else
        r2File = "Cov_oyster_to.csv";

    std::ofstream out_file;
    out_file.open (covFile.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++) {
        for (int j=0; j<numStations; j++) {
            out_file << cov (i, j);
            if (j == (numStations - 1))
                out_file << std::endl;
            else
                out_file << ", ";
        }
    }
    out_file.close ();
    std::cout << "Covariances written to " << covFile.c_str () << std::endl;

    return 0;
}

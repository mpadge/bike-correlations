#include "TrainData.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          READOYSTERDATA                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::readOysterData ()
{
    /*
     * At the moment, tube trips are only taken as "LUL", and not "DLR" or
     * "LUL/DLR".
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
    bool startIn, stopIn; // TODO: DELETE THESE!

    /*
    for (std::vector <OneStation>::iterator itr = StationList.begin();
            itr != StationList.end(); itr++)
        std::cout << "->" << (*itr).name << "<-" << std::endl;
    */

    if (_tube)
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
                    // TODO: INSERT ERROR HANDLER
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
        // TODO: INSERT ERROR HANDLER
        return -1;
    }
    in_file.clear ();
    in_file.seekg (0);
    getline (in_file, linetxt, '\n');
    int nlines = 0;
    while (getline (in_file, linetxt, '\n'))
        nlines++;
    in_file.clear ();
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n');

    std::cout << "Reading oystercarddata for " << StationList.size () <<
        " " << modeTxt << " stations ... ";
    std::cout.flush ();
    int progress [2] = {0, 1};
    while (getline (in_file, linetxt, '\n')) { 
        ipos = linetxt.find("\",\"",0);
        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        ipos = linetxt.find("\",\"",0);
        mode = linetxt.substr (0, ipos);
        if ((_tube && mode == "LUL") || (!_tube && mode == "NR"))
        {
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            start = standardise (linetxt.substr (0, ipos));
            start = substituteNames (_tube, start);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find("\",\"",0);
            stop = standardise (linetxt.substr (0, ipos));
            stop = substituteNames (_tube, stop);


            if (((_tube && mode == "LUL") || (!_tube && mode == "NR")) &&
                (start != "unstarted" && stop != "unfinished" && 
                    start != "bus" && stop != "bus" &&
                    start != "not applicable" && stop != "not applicable"))
            {
                // First look for start and stop in the Oyster2StnIndex
                tempi [0] = tempi [1] = INT_MIN;
                count = 0;
                for (std::vector <OysterIndx>::iterator
                        itr = Oyster2StnIndex.begin();
                        itr != Oyster2StnIndex.end(); itr++)
                {
                    if ((*itr).name == start)
                        tempi [0] = (*itr).index;
                    if ((*itr).name == stop)
                        tempi [1] = (*itr).index;
                    count++;
                } // end itr over Oyster2StnIndex
                if (tempi [0] == INT_MIN) // Add start to Oyster2StnIndex
                {
                    count = 0;
                    for (std::vector <OneStation>::iterator 
                            itr = StationList.begin();
                            itr != StationList.end(); itr++)
                    {
                        if ((*itr).name == start)
                            tempi [0] = count;
                        count++;
                    }
                    if (tempi [0] == INT_MIN)
                    {
                        std::cout << "WARNING: <" << start <<
                            "> not in StationList" << std::endl;
                    }
                    else
                        Oyster2StnIndex.push_back ({start, tempi [0]});
                }
                if (tempi [1] == INT_MIN && stop != start)
                {
                    count = 0;
                    for (std::vector <OneStation>::iterator 
                            itr = StationList.begin();
                            itr != StationList.end(); itr++)
                    {
                        if ((*itr).name == stop)
                            tempi [1] = count;
                        count++;
                    }
                    if (tempi [1] == INT_MIN)
                    {
                        //std::cout << "WARNING: <" << start <<
                        //    "> not in StationList" << std::endl;
                    }
                    else
                        Oyster2StnIndex.push_back ({stop, tempi [1]});
                }
                if (tempi [0] > INT_MIN && tempi [1] > INT_MIN)
                    ntrips (tempi [0], tempi [1]) += 1.0;
            } // end if start and stop valid
        } // end if mode valid
        progress [0]++;
        if (floor (100.0 * (double) progress [0] / (double) nlines) >
                progress [1])
        {
            std::cout << "\rReading oystercarddata for " << StationList.size () <<
                " " << modeTxt << " stations ... " << progress [1] << "%";
            std::cout.flush ();
            progress [1]++;
        }
        //if (progress [1] > 2)
        //    break;
    } // end while getline
    in_file.close();
    remove (fname_csv.c_str ());
    std::cout << "\rReading oystercarddata for " << StationList.size () <<
        " " << modeTxt << " stations ... 100\% done." << std::endl;

    std::sort (Oyster2StnIndex.begin(), Oyster2StnIndex.end(),
            [] (OysterIndx a, OysterIndx b) { return a.name < b.name; });

    return 0;
} // end readOysterData


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           FILLHASDATA                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::fillHasData ()
{
    int count = 0;
    bool check;
    for (int i=0; i<_numStations; i++)
    {
        check = false;
        for (int j=0; j<_numStations; j++)
            if (ntrips (i, j) > 0.0 || ntrips (j, i) > 0.0)
                check = true;
        hasData.push_back (check);
        if (check)
            count++;
    }

    return count;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          REDUCENTRIPS                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int TrainData::resizeNtrips ()
{
    // Actually resizes all StationData matrices (ntrips, r2, cov, dists)
    int indx [2] = {0, 0}, nSt = StationList.size ();
    dmat ntemp (numStnsWithData, numStnsWithData);
    // TODO: Rewrite this in a more robust way. At present just uses the
    // following simple sanity check.
    for (std::vector<bool>::iterator itr = hasData.begin ();
            itr != hasData.end(); itr++)
        if (*itr)
            indx [0]++;
    if (indx [0] != numStnsWithData)
    {
        std::cout << "ERROR: hasData includes " << indx [0] << 
            " but numStnsWithData = " << numStnsWithData << std::endl;
        return -1;
    }

    indx [0] = 0;
    for (int i=0; i<nSt; i++)
    {
        if (hasData [i])
        {
            indx [1] = 0;
            for (int j=0; j<nSt; j++)
                if (hasData [j])
                {
                    ntemp (indx [0], indx [1]) = ntrips (i, j);
                    indx [1]++;
                }
            indx [0]++;
        }
    } // end for i

    // Then read back into resized ntrips
    ntrips.resize (numStnsWithData, numStnsWithData);
    for (int i=0; i<numStnsWithData; i++)
        for (int j=0; j<numStnsWithData; j++)
            ntrips (i, j) = ntemp (i, j);
    ntemp.resize (0, 0);
    r2.resize (numStnsWithData, numStnsWithData);
    cov.resize (numStnsWithData, numStnsWithData);
    dists.resize (numStnsWithData, numStnsWithData);
}

#include "RideData.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          COUNTFILESLONDON                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::countFilesLondon (int filei)
{
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err;

    std::string fname_base = StationData::GetDirName() + '/' + filelist [filei];
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
            zf = zip_fopen_index(za, 0, 0);
            if (!zf) {
                std::cout << stderr << "ERROR: cannot open file#" <<
                    i << " in archive " << archive << std::endl;
                return 1;
            }
            zip_fclose(zf); 
        }
    }
    zip_close (za);

    return zip_get_num_entries (za, 0);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETFILENAMESLONDON                         **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::unzipOneFileLondon (int filei, int filej)
{
    // Unzips file#j from the zip archive file#i in the directory
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, sum, len;

    std::ofstream out_file;
    std::string fname_csv, fname_base = StationData::GetDirName() + '/' + filelist [filei];

    archive = fname_base.c_str ();
    // Error checks already done with countFilesLondon
    za = zip_open(archive, 0, &err);
    if (zip_stat_index (za, filej, 0, &sb) == 0) {
        fileName = sb.name;
        zf = zip_fopen_index(za, filej, 0);

        fname_csv = StationData::GetDirName() + '/' + fileName;
        out_file.open (fname_csv.c_str(), std::ios::out);
        sum = 0;
        while (sum != sb.size) {
            len = zip_fread(zf, buf, 100);
            if (len < 0) {
                // INSERT ERROR HANDLER
            }
            out_file.write (buf, len);
            sum += len;
            //std::cout << sum << std::endl;
        }
        out_file.close ();

        zip_fclose(zf); 
    }
    zip_close(za);

    return 0;
}

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         READONEFILELONDON                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileLondon ()
{
    bool alreadyMissing;
    int ID, count = 0, ipos, tempi [2];
    int nstations = RideData::getNumStations (), 
        maxStation = RideData::getMaxStation ();
    std::string fname = StationData::GetDirName() + '/' + fileName;
    std::ifstream in_file;
    std::string linetxt;

    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // INSERT ERROR HANDLER
        return -1;
    }
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');

    while (getline (in_file, linetxt, '\n')) { 
        ipos = linetxt.find(',',0);
        ID = atoi (linetxt.substr (0, ipos).c_str()); // Trip ID number
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        for (int j=0; j<3; j++) {
            ipos = linetxt.find(',',0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (',', 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        for (int j=0; j<2; j++) {
            ipos = linetxt.find ('"', 0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        for (int j=0; j<2; j++) {
            ipos = linetxt.find(',',0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find(',',0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID

        if (tempi [1] > 0 && tempi [0] <= maxStation && tempi [1] > 0 &&
                tempi [1] <= maxStation)
        {
            ipos = -1;
            if (_StationIndex [tempi [0]] == INT_MIN)
                ipos = tempi [0];
            else if (_StationIndex [tempi [1]] == INT_MIN)
                ipos = tempi [1];
            if (ipos > -1)
            {
                alreadyMissing = false;
                for (std::vector <int>::iterator pos=missingStations.begin();
                        pos != missingStations.end(); pos++)
                    if (*pos == ipos)
                        alreadyMissing = true;
                if (!alreadyMissing)
                    missingStations.push_back (ipos);
            } else {
                tempi [0] = _StationIndex [tempi[0]];
                tempi [1] = _StationIndex [tempi[1]];
                ntrips (tempi [1], tempi [0]) += 1.0;
                count++;
            }
        }
    } // end while getline
    in_file.close();

    return count;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                        DUMPMISSINGSTATIONS                         **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::dumpMissingStations ()
{
    if (missingStations.size () > 0)
    {
        std::sort (missingStations.begin(), missingStations.end());
        std::cout << "The following stations are in trip files " <<
            "yet missing from station_latlons:" << std::endl << "(";
        for (std::vector <int>::iterator pos=missingStations.begin();
                pos != missingStations.end(); pos++)
            std::cout << *pos << ", ";
        std::cout << ")" << std::endl;
    }
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         READONEFILELONDON                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::removeFile ()
{
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;

    if (remove(fname_csv.c_str()) != 0)
        return 1;
    else
        return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         GETZIPFILENAMENYC                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::getZipFileNameNYC (int filei)
{
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err;

    std::string fname_base = StationData::GetDirName() + '/' + filelist [filei];
    archive = fname_base.c_str ();
    if ((za = zip_open(archive, 0, &err)) == NULL) {
        zip_error_to_str(buf, sizeof(buf), err, errno);
        std::cout << stderr << archive << "can't open size archive : " <<
            buf << std::endl;
        return 1;
    } 

    if (zip_get_num_entries (za, 0) == 1) {
        if (zip_stat_index(za, 0, 0, &sb) == 0) {
            fileName = sb.name;
            zf = zip_fopen_index(za, 0, 0);
            if (!zf) {
                fprintf(stderr, "ERROR: zip can not be opened/n");
                return 1;
            }
        }
    } 
    zip_fclose(zf); 
    if (zip_close(za) == -1) {
        std::cout << stderr << "can't close zip archive " << archive <<
            std::endl;
        fprintf(stderr, "can't close zip archive `%s'/n", archive);
        return 1;
    }

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           READONEFILENYC                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileNYC (int filei)
{
    // First unzip file, for which all error checks have been done in
    // getZipFileNameNYC above
    std::string fname = StationData::GetDirName() + '/' + filelist [filei];
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len;
    long long sum;

    archive = fname.c_str ();
    za = zip_open(archive, 0, &err);
    zf = zip_fopen_index(za, 0, 0);
    zip_stat_index(za, 0, 0, &sb);
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;
    std::ofstream out_file (fname_csv.c_str(), std::ios::out);

    sum = 0;
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

    // Then read unzipped .csv file
    int count = 0, ipos, tempi [2];
    int nstations = getStnIndxLen ();
    std::ifstream in_file;
    std::string linetxt;

    in_file.open (fname_csv.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // INSERT ERROR HANDLER
    } 

    getline (in_file, linetxt, '\n');
    while (getline (in_file, linetxt, '\n')) { count++;	}

    std::cout << "Reading file [";
    if (filei < 10)
        std::cout << " ";
    std::cout << filei << "/" << filelist.size() <<
        "]: " << fileName.c_str() << " with " <<
        count << " records";
    std::cout.flush ();

    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');
    count = 0;

    /* Structure is:
    [1] tripduration, [2] starttime, [3] stoptime, [4] start station id, 
    [5] start station name, [6] start station latitude, 
    [7] start station longitude, [8] end station id, [9] end station name, 
    [10] end station latitude [11] end station longitude, [12] bikeid, 
    [13] usertype, [14] birth year, [15] gender
    */
    while (getline (in_file, linetxt,'\n')) {
        for (int i=0; i<3; i++) {
            ipos = linetxt.find("\",\"",0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find ("\",\"", 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        for (int i=0; i<3; i++) {
            ipos = linetxt.find ("\",\"", 0);
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find ("\",\"", 0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
        if (tempi [0] >= 0 && tempi [0] < RideData::getStnIndxLen() && 
                tempi [1] >= 0 && tempi [1] < RideData::getStnIndxLen() &&
                tempi [0] != tempi [1])
        {
            tempi [0] = _StationIndex [tempi[0]];
            tempi [1] = _StationIndex [tempi[1]];
            if (tempi [0] >= 0 && tempi [0] < nstations && tempi [1] >= 0 && 
                    tempi [1] < nstations) {
                ntrips (tempi [0], tempi [1]) += 1.0;
                count++; 
            } // end if 
        }
    } // end while getline
    in_file.close();
    std::cout << " and " << count << " valid trips." << std::endl;

    return count;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           WRITENUMTRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::writeNumTrips ()
{
    int numStations = RideData::getNumStations ();
    std::string fname;
    if (RideData::getCity() == "london")
        fname = "NumTrips_london.csv";
    else
        fname = "NumTrips_nyc.csv";

    std::ofstream out_file;
    out_file.open (fname.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++) {
        for (int j=0; j<numStations; j++) {
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

int RideData::calcR2 (bool from, bool standardise)
{
    int numStations = RideData::getNumStations ();
    double tempd [2];
    std::vector <double> x, y;
    RegrResults regrResults;

    y.resize (0);
    r2.resize (numStations, numStations);
    for (int i=0; i<numStations; i++) {
        for (int j=0; j<numStations; j++) {
            r2 (i, j) = -9999.9;
            cov (i, j) = -9999.9;
        }
    }

    for (int i=0; i<(numStations-1); i++) {
        x.resize (0);
        for (int j=0; j<numStations; j++)
            if (from)
                x.push_back (ntrips (i, j));
            else
                x.push_back (ntrips (j, i));
        for (int j=(i+1); j<numStations; j++) {
            y.resize (0);
            for (int k=0; k<numStations; k++)
                if (from)
                    y.push_back(ntrips (j, k));
                else
                    y.push_back(ntrips (k, j));

            if (standardise)
            {
                tempd [0] = tempd [1] = 0.0;
                // Avoid using boost for double iterator here for clarity
                for (int k=0; k<x.size(); k++)
                {
                    tempd [0] += x[k];
                    tempd [1] += y[k];
                }
                for (int k=0; k<x.size(); k++)
                {
                    x[k] = x[k] / tempd [0];
                    y[k] = y[k] / tempd [1];
                }
            }
            regrResults = regression (x, y);
            r2 (i, j) = r2 (j, i) = regrResults.r2;
            cov (i, j) = cov (j, i) = regrResults.cov;
        }
    }
    x.resize (0);
    y.resize (0);

    return 0;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             WRITER2MAT                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::writeR2Mat (bool from)
{
    int numStations = RideData::getNumStations ();
    std::string r2File, stdext;
    if (RideData::getCity() == "london")
        if (from) 
            r2File = "R2_from_london.csv";
        else
            r2File = "R2_to_london.csv";
    else
        if (from)
            r2File = "R2_from_nyc.csv";
        else
            r2File = "R2_to_nyc.csv";

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

int RideData::writeCovMat (bool from)
{
    int numStations = RideData::getNumStations ();
    std::string covFile, stdext;
    bool standardise = RideData::getStandardise ();
    if (standardise)
        stdext = "_standardised";
    else
        stdext = "_unstandardised";

    if (RideData::getCity() == "london")
        if (from)
            covFile = "Cov_from_london" + stdext + ".csv";
        else
            covFile = "Cov_to_london" + stdext + ".csv";
    else
        if (from)
            covFile = "Cov_from_nyc" + stdext + ".csv";
        else
            covFile = "Cov_to_nyc" + stdext + ".csv";

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

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                             READR2MAT                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readR2Mat (bool from)
{
    // TODO: Delete this whole routine!
    int count, ipos;
    double tempd;
    std::string linetxt;
    int numStations = RideData::getNumStations ();
    std::string r2File;
    if (from)
        r2File = "R2_from.csv";
    else
        r2File = "R2_to.csv";

    r2.resize (numStations, numStations);
    std::ifstream in_file;
    in_file.open (r2File.c_str (), std::ofstream::in);
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    count = 0;
    while (getline (in_file, linetxt, '\n')) {
        for (int i=0; i<(numStations - 1); i++) {
            ipos = linetxt.find(',',0);
            tempd = atof (linetxt.substr (0, ipos).c_str()); 
            r2 (count, i) = r2 (i, count) = tempd;
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        tempd = atof (linetxt.c_str());
        r2 (count, numStations - 1) = r2 (numStations - 1, count) = tempd;
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

int RideData::writeDMat ()
{
    std::string city = RideData::getCity(), distFile;
    if (city == "london")
        distFile = "results/station_dists_london.txt";
    else if (city == "nyc")
        distFile = "results/station_dists_nyc.txt";

    int numStations = RideData::getNumStations ();

    int count, ipos, tempi [2];
    double d;
    std::ifstream in_file;
    std::string linetxt;
    in_file.open (distFile.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // INSERT ERROR HANDLER
    } 
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    count = 0;
    while (getline (in_file, linetxt, '\n')) {
        ipos = linetxt.find(',',0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find(',',0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID
        tempi [0] = _StationIndex [tempi[0]];
        tempi [1] = _StationIndex [tempi[1]];
        if (tempi[0] < 0 | tempi[0] > numStations)
            std::cout << "ERROR: stn[0]#" << tempi[0] << std::endl;
        if (tempi[1] < 0 | tempi[1] > numStations)
            std::cout << "ERROR: stn[1]#" << tempi[1] << std::endl;
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        d = atof (linetxt.c_str());
        dists (tempi [0], tempi [1]) = dists (tempi [1], tempi [0]) = d;
        count++;
    }
    in_file.close ();

    if (city == "london")
        distFile = "stationDistsMat_london.csv";
    else if (city == "nyc")
        distFile = "stationDistsMat_nyc.csv";
    std::ofstream out_file;
    out_file.open (distFile.c_str (), std::ofstream::out);
    for (int i=0; i<numStations; i++) {
        for (int j=0; j<numStations; j++) {
            out_file << dists (i, j);
            if (j == (numStations - 1))
                out_file << std::endl;
            else
                out_file << ", ";
        }
    }
    out_file.close ();
    dists.resize (0,0);
    std::cout << "Inter-station distances written to " << 
        distFile.c_str () << std::endl;

    return 0;
}

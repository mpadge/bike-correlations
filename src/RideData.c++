#include "RideData.h"

/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         READONEFILELONDON                          **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::readOneFileLondon (int filei)
{
    int count, ipos, tempi [2];
    int nstations = getNumStations ();
    std::string fname = StationData::GetDirName() + '/' + filelist [filei];
    std::ifstream in_file;
    std::string linetxt;

    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // INSERT ERROR HANDLER
    } 
    std::cout << "Reading file [";
    if (filei < 10)
        std::cout << " ";
    std::cout << filei << "/" << filelist.size() <<
        "]: " << filelist [filei];
    std::cout.flush ();

    getline (in_file, linetxt, '\n');
    count = 0;
    while (getline (in_file, linetxt, '\n')) { count++;	}
    in_file.clear ();
    in_file.seekg (0); // Both lines needed to rewind file.
    getline (in_file, linetxt, '\n');

    for (int k=0; k<=count; k++) {
        getline (in_file, linetxt,'\n');
        for (int j=0; j<4; j++) {
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

        if (k < 10) 
            std::cout << "[" << tempi[0] << "," << tempi[1] << "] -> [";
        tempi [0] = _StationIndex [tempi[0]];
        tempi [1] = _StationIndex [tempi[1]];
        if (k < 10)
            std::cout << "[" << tempi[0] << "," << tempi[1] << "]" << std::endl;
        if (tempi [0] > 0 && tempi [0] <= nstations && tempi [1] > 0 && 
                tempi [1] <= nstations) {
            ntrips (tempi [1] - 1, tempi [0] - 1)++;
        } // end if 
    } // end for k
    in_file.close();
    std::cout << " done." << std::endl;
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
                ntrips (tempi [0], tempi [1])++;
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
 **                           REMOVEFILENYC                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::removeFileNYC ()
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
 **                           WRITENUMTRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::writeNumTrips ()
{
    int numStations = RideData::getNumStations ();
    std::string fname = "NumTrips.csv";

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

int RideData::calcR2 (bool from)
{
    int numStations = RideData::getNumStations ();
    double tempd [3];
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
    std::string r2File;
    if (from)
        r2File = "R2_from.csv";
    else
        r2File = "R2_to.csv";

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
    std::string covFile;
    if (from)
        covFile = "Cov_from.csv";
    else
        covFile = "Cov_to.csv";

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
        distFile = "../results/station_dists_london.txt";
    else if (city == "nyc")
        distFile = "../results/station_dists_nyc.txt";

    int nstations = getNumStations ();
    int numStations = RideData::getNumStations ();
    for (int i=0; i<numStations; i++)
        dists (i, i) = 0.0;

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

    distFile = "stationDists.csv";
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

    return 0;
}

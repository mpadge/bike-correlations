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
 **                         UNZIPONEFILELONDON                         **
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
                // TODO: INSERT ERROR HANDLER
            }
            out_file.write (buf, len);
            sum += len;
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
    int maxStation = RideData::returnMaxStation ();
    std::string fname = StationData::GetDirName() + '/' + fileName;
    std::ifstream in_file;
    std::string linetxt;

    in_file.open (fname.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // TODO: INSERT ERROR HANDLER
        return -1;
    }
    in_file.clear ();
    in_file.seekg (0); 
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
 **                            REMOVEFILE                              **
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
    int err, len, fileYear, age;
    long long sum;

    archive = fname.c_str ();
    za = zip_open(archive, 0, &err);
    zf = zip_fopen_index(za, 0, 0);
    zip_stat_index(za, 0, 0, &sb);
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;
    std::ofstream out_file (fname_csv.c_str(), std::ios::out);
    fileYear = atoi (fileName.substr (0, 4).c_str());

    sum = 0;
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

    // Then read unzipped .csv file
    int count = 0, ipos, tempi [4];
    int nstations = getStnIndxLen ();
    std::ifstream in_file;
    std::string linetxt, usertype;

    in_file.open (fname_csv.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // TODO: INSERT ERROR HANDLER
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
    in_file.seekg (0); 
    getline (in_file, linetxt, '\n');
    count = 0;

    /* Structure is:
    [1] tripduration, [2] starttime, [3] stoptime, [4] start station id, 
    [5] start station name, [6] start station latitude, 
    [7] start station longitude, [8] end station id, [9] end station name, 
    [10] end station latitude [11] end station longitude, [12] bikeid, 
    [13] usertype, [14] birth year, [15] gender (1=male, 2=female)
    Note that birthyears seem uncontrolled, so there are quite a number that are
    1899, 1900, or 1901, yet none further until 1920 or so.
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
            if (tempi [0] < 0 || tempi [0] > nstations || tempi [1] < 0 ||
                    tempi [1] > nstations) { // should never happen
                std::cout << "ERROR: station not in StationIndex!" << std::endl;
            }
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            // Then extract usertype, birthyear, gender
            for (int i=0; i<4; i++) {
                ipos = linetxt.find("\",\"",0);
                linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            }
            ipos = linetxt.find ("\",\"", 0);
            usertype = linetxt.substr (0, ipos).c_str(); // User type
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find ("\",\"", 0);
            tempi [2] = atoi (linetxt.substr (0, ipos).c_str()); // Birthyear
            age = fileYear - tempi [2];
            if (age > 0 && age < 99)
                ageDistribution [age]++;
            tempi [2] = floor (tempi [2] / 10);
            if (RideData::getSubscriber () > 2)
            {
                if (age > 0 && age < 38) // Average age is 37.7
                    ntripsYoung (tempi [0], tempi [1])++;
                else if (age < 99)
                    ntripsOld (tempi [0], tempi [1])++;
                // TODO: Write this better!
                if (tempi [2] == 192)
                    ntrips1920 (tempi [0], tempi [1])++;
                else if (tempi [2] == 193)
                    ntrips1930 (tempi [0], tempi [1])++;
                else if (tempi [2] == 194)
                    ntrips1940 (tempi [0], tempi [1])++;
                else if (tempi [2] == 195)
                    ntrips1950 (tempi [0], tempi [1])++;
                else if (tempi [2] == 196)
                    ntrips1960 (tempi [0], tempi [1])++;
                else if (tempi [2] == 197)
                    ntrips1970 (tempi [0], tempi [1])++;
                else if (tempi [2] == 198)
                    ntrips1980 (tempi [0], tempi [1])++;
                else if (tempi [2] == 199)
                    ntrips1990 (tempi [0], tempi [1])++;
                else if (tempi [2] == 200)
                    ntrips2000 (tempi [0], tempi [1])++;
            }
            else
            {
                linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
                tempi [3] = atoi (linetxt.substr (0, 1).c_str()); // Gender
                if (usertype != "Subscriber")
                    ntrips_cust (tempi [0], tempi [1])++;
                else
                    if (tempi [3] == 1)
                        ntrips_sub_m (tempi [0], tempi [1])++;
                    else if (tempi [3] == 2)
                        ntrips_sub_f (tempi [0], tempi [1])++;
                    else
                        ntrips_sub_n (tempi [0], tempi [1])++;
            }
            count++; 
        } // end if stations in StnIndxLen
    } // end while getline
    in_file.close();
    std::cout << " and " << count << " valid trips." << std::endl;

    return count;
} // end readOneFileNYC


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          SUMMARYSTATSNYC                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::summaryStatsNYC ()
{
    // Average Age
    int tempi [2] = {0,0};
    for (int i=0; i<99; i++)
    {
        tempi [0] += i * ageDistribution [i];
        tempi [1] += ageDistribution [i];
    }
    std::cout << "Average Age = " << (double) tempi [0] / (double) tempi [1] << 
        std::endl;

    if (RideData::getSubscriber () < 3)
    {
        // Male-Female ratio
        tempi [0] = tempi [1] = 0;
        int numStations = RideData::returnNumStations ();
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
            {
                tempi [0] += ntrips_sub_f (i, j);
                tempi [1] += ntrips_sub_m (i, j);
            }
        std::cout << "Female/Male ratio = " << (double) tempi [0] /
            (double) tempi [1] << std::endl;
    }
} // end summaryStatsNYC


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          AGGREGATETRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::aggregateTrips ()
{
    int numStations = RideData::returnNumStations (),
        subscriber = RideData::getSubscriber (),
        gender = RideData::getGender ();
    // subscriber = (0, 1, 2) for (all, subscriber, customer)
    // gender = (0, 1, 2) for (all, male, female)
    // (male, female) only make sense for subscriber = 1, and are ignored
    // otherwise.
    if (subscriber == 0)
    {
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
                ntrips (i, j) += (double) ntrips_sub_f (i, j) +
                    (double) ntrips_sub_m (i, j) +
                    (double) ntrips_sub_n (i, j) +
                    (double) ntrips_cust (i, j);

    }
    else if (subscriber == 1)
    {
        if (gender == 0)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_f (i, j) +
                        (double) ntrips_sub_m (i, j);
        else if (gender == 1)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_m (i, j);
        else if (gender == 2)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips_sub_f (i, j);
    }
    else if (subscriber == 2)
    {
        for (int i=0; i<numStations; i++)
            for (int j=0; j<numStations; j++)
                ntrips (i, j) += (double) ntrips_cust (i, j);
    }
    else
    {
        if (gender == 0) // "young"
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntripsYoung (i, j);
        else if (gender == 1) // "old"
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntripsOld (i, j);
        else if (gender == 1920)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1920 (i, j);
        else if (gender == 1930)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1930 (i, j);
        else if (gender == 1940)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1940 (i, j);
        else if (gender == 1950)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1950 (i, j);
        else if (gender == 1960)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1960 (i, j);
        else if (gender == 1970)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1970 (i, j);
        else if (gender == 1980)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1980 (i, j);
        else if (gender == 1990)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1990 (i, j);
        else if (gender == 2000)
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips2000 (i, j);
        else // default should not happen!
            for (int i=0; i<numStations; i++)
                for (int j=0; j<numStations; j++)
                    ntrips (i, j) += (double) ntrips1920 (i, j) +
                    (double) ntrips1930 (i, j) + (double) ntrips1940 (i, j) +
                    (double) ntrips1950 (i, j) + (double) ntrips1960 (i, j) +
                    (double) ntrips1970 (i, j) + (double) ntrips1980 (i, j) +
                    (double) ntrips1990 (i, j) + (double) ntrips2000 (i, j);
    }
} // end aggregateTrips


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           WRITENUMTRIPS                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::writeNumTrips ()
{
    int numStations = RideData::returnNumStations ();
    std::string fname;
    if (RideData::returnCity() == "london")
        fname = "NumTrips_london.csv";
    else
        fname = "NumTrips_nyc_" +
            std::to_string (RideData::getSubscriber ()) + 
            std::to_string (RideData::getGender ()) + ".csv";

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

int RideData::calcR2 (bool from)
{
    bool standardise = RideData::getStandardise();
    int tempi, numStations = RideData::returnNumStations ();
    double tempd;
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
            tempd = 0.0;
            for (int j=0; j<numStations; j++)
                tempd += x0 [j];
            for (int j=0; j<numStations; j++)
                x0 [j] = x0 [j] / tempd;
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
                tempd = 0.0;
                for (int k=0; k<numStations; k++)
                    tempd += y0 [k];
                for (int k=0; k<numStations; k++)
                    y0 [k] = y0 [k] / tempd;
            }

            if (nearfar != 0) // Remove half of stations from lists
            {
                d.resize (0);
                for (int k=0; k<numStations; k++)
                    d.push_back (dists (i, k) + dists (j, k));
                std::sort (d.begin(), d.end());
                tempi = floor (d.size () / 2);
                tempd = (d [tempi] + d [tempi + 1]) / 2.0;
                d.resize (0);
                for (int k=0; k<numStations; k++)
                    d.push_back (dists (i, k) + dists (j, k));
                x2.resize (0);
                y2.resize (0);
                for (int k=0; k<numStations; k++)
                {
                    if (nearfar == 1 && d[k] < tempd)
                    {
                        x2.push_back (x1[k]);
                        y2.push_back (y0[k]);
                    } else if (nearfar == 2 && d[k] > tempd)
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

int RideData::writeR2Mat (bool from)
{
    int numStations = RideData::returnNumStations (),
        subscriber = RideData::getSubscriber (),
        gender = RideData::getGender ();
    std::string r2File;
    if (RideData::returnCity() == "london")
        if (from) 
            r2File = "R2_london_from_" + txtnf + ".csv";
        else
            r2File = "R2_london_to_" + txtnf + ".csv";
    else
        if (from)
            r2File = "R2_nyc_from_" + txtnf + "_" + 
                std::to_string (RideData::getSubscriber ()) + 
                std::to_string (RideData::getGender ()) + ".csv";
        else
            r2File = "R2_nyc_to_" + txtnf + "_" + 
                std::to_string (RideData::getSubscriber ()) + 
                std::to_string (RideData::getGender ()) + ".csv";

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
    int numStations = RideData::returnNumStations ();
    std::string covFile, stdext;
    bool standardise = RideData::getStandardise ();
    if (standardise)
        stdext = "_std";
    else
        stdext = "_unstd";

    if (RideData::returnCity() == "london")
        if (from)
            covFile = "Cov_london_from" + stdext + "_" + txtnf + ".csv";
        else
            covFile = "Cov_london_to" + stdext + "_" + txtnf + ".csv";
    else
        if (from)
            covFile = "Cov_nyc_from" + stdext + "_" + txtnf + "_" +
                std::to_string (RideData::getSubscriber ()) + 
                std::to_string (RideData::getGender ()) + ".csv";
        else
            covFile = "Cov_nyc_to" + stdext + "_" + txtnf + "_" +
                std::to_string (RideData::getSubscriber ()) + 
                std::to_string (RideData::getGender ()) + ".csv";

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
    int numStations = RideData::returnNumStations ();
    std::string r2File;
    if (from)
        r2File = "R2_from.csv";
    else
        r2File = "R2_to.csv";

    r2.resize (numStations, numStations);
    std::ifstream in_file;
    in_file.open (r2File.c_str (), std::ofstream::in);
    in_file.clear ();
    in_file.seekg (0); 
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
    std::string city = RideData::returnCity(), distFile;
    if (city == "london")
        distFile = "results/station_dists_london.txt";
    else if (city == "nyc")
        distFile = "results/station_dists_nyc.txt";

    int numStations = RideData::returnNumStations ();

    int count, ipos, tempi [2];
    double d;
    std::ifstream in_file;
    std::string linetxt;
    in_file.open (distFile.c_str (), std::ifstream::in);
    if (in_file.fail ()) {
        // TODO: INSERT ERROR HANDLER
    } 
    in_file.clear ();
    in_file.seekg (0); 
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
    std::cout << "Inter-station distances written to " << 
        distFile.c_str () << std::endl;

    return 0;
}

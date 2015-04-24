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
 **                           SUMMARYSTATS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void RideData::summaryStats ()
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
} // end summaryStats


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                        GETZIPFILENAMEBOSTON                        **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::getZipFileNameBoston (int filei)
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

    if (zip_stat_index(za, 0, 0, &sb) == 0) {
        fileName = sb.name;
        assert (fileName == "hubway_trips.csv");
        zf = zip_fopen_index(za, 0, 0);
        if (!zf) {
            fprintf(stderr, "ERROR: zip can not be opened/n");
            return 1;
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
 **                          READONEFILEBOSTON                         **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileBoston (int filei)
{
    // First unzip file, for which all error checks have been done in
    // getZipFileNameBoston above
    std::string fname = StationData::GetDirName() + '/' + filelist [filei];
    const char *archive;
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[100]; 
    int err, len, fileYear = 2013, age;
    long long sum;

    archive = fname.c_str ();
    za = zip_open(archive, 0, &err);
    zf = zip_fopen_index(za, 0, 0);
    zip_stat_index(za, 0, 0, &sb);
    std::string fname_csv = StationData::GetDirName() + '/' + fileName;
    assert (fileName == "hubway_trips.csv");
    // TODO: Replace the 0 index in zip_fopen_index and zip_stat_index with
    // appropriate numbers to avoid the above assert.
    std::ofstream out_file (fname_csv.c_str(), std::ios::out);

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
     * [1] seq_id, [2] hubway_id, [3] status, [4] duration, [5] start_date,
     * [6] strt_statn, [7] end_date, [8] end_statn, [9] bike_nr, 
     * [10] subsc_type, [11] zip_code, [12] birth_date, [13] gender
     */
    while (getline (in_file, linetxt,'\n')) {
        for (int i=0; i<5; i++) {
            ipos = linetxt.find(",",0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (",", 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (",", 0);
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (",", 0);
        tempi [1] = atoi (linetxt.substr (0, ipos).c_str()); // End Station ID

        /* NOTE: Missing stations return 0 from atoi, and there is no station#0
         * for Boston.
         * TODO: Check that!
         */
        if (tempi [0] > 0 && tempi [0] < RideData::getStnIndxLen() && 
                tempi [1] > 0 && tempi [1] < RideData::getStnIndxLen() &&
                tempi [0] != tempi [1])
        {
            tempi [0] = _StationIndex [tempi[0]];
            tempi [1] = _StationIndex [tempi[1]];
            if (tempi [0] < 0 || tempi [0] > nstations || tempi [1] < 0 ||
                    tempi [1] > nstations) { // should never happen
                std::cout << "ERROR: station [" << 
                    tempi [0] << "->" << tempi[1] << "] not in StationIndex!" <<
                    std::endl;
            }
            linetxt = linetxt.substr (ipos + 3, linetxt.length () - ipos - 1);
            ipos = linetxt.find(",",0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (",", 0);
            usertype = linetxt.substr (0, ipos).c_str(); // User type
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (",", 0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find(",",0);
            tempi [2] = atoi (linetxt.substr (0, ipos).c_str()); // Birthyear
            age = fileYear - tempi [2];
            if (age > 0 && age < 99)
                ageDistribution [age]++;
            tempi [2] = floor (tempi [2] / 10);
            if (RideData::getSubscriber () > 2)
            {
                if (age > 0 && age < 37) // Average age is 36.7
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
                linetxt = linetxt.substr (ipos + 1, 1);
                if (usertype != "Registered")
                    ntrips_cust (tempi [0], tempi [1])++;
                else
                    if (linetxt == "M")
                        ntrips_sub_m (tempi [0], tempi [1])++;
                    else if (linetxt == "F")
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
 **                       GETZIPFILENAMECHICAGO                        **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::getZipFileNameChicago (int filei)
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
 **                         READONEFILECHICAGO                         **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileChicago (int filei)
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

    // Note files are presumed all to start with "Divvy_Trips_"!
    fileYear = 2000 + atoi (fileName.substr (13, 4).c_str());

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
    std::string linetxt, usertype, gender;

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
     *
     * [1] trip_id, [2] starttime, [3] stoptime, [4] bikeid, [5] tripduration,
     * [6] from_station_id, [7] from_station_name, [8] to_station_id,
     * [9] to_station_name, [10] usertype, [11] gender, [12] birthyear
     *
    */
    while (getline (in_file, linetxt,'\n')) {
        for (int i=0; i<5; i++) {
            ipos = linetxt.find(",",0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (",", 0);
        tempi [0] = atoi (linetxt.substr (0, ipos).c_str()); // Start Station ID
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (",", 0);
        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        ipos = linetxt.find (",", 0);
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
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            // Then extract usertype, birthyear, gender
            ipos = linetxt.find(",",0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (",", 0);
            usertype = linetxt.substr (0, ipos).c_str(); // User type
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            ipos = linetxt.find (",", 0);
            gender = linetxt.substr (0, ipos).c_str ();

            if (usertype != "Subscriber")
                ntrips_cust (tempi [0], tempi [1])++;
            else
                if (gender.empty ())
                    ntrips_sub_n (tempi [0], tempi [1])++;
                else if (gender.substr (0, 1) == "M")
                    ntrips_sub_m (tempi [0], tempi [1])++;
                else if (gender.substr (0, 1) == "F")
                    ntrips_sub_f (tempi [0], tempi [1])++;

            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
            // linetxt is then last entry of Age
            
            if (!linetxt.empty ())
            {
                tempi [2] = atoi (linetxt.c_str()); // Birthyear
                age = fileYear - tempi [2];
                if (age > 0 && age < 99)
                    ageDistribution [age]++;
                tempi [2] = floor (tempi [2] / 10);

                if (age > 0 && age < 36) // Average age is 35.5
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
            count++; 
        } // end if stations in StnIndxLen
    } // end while getline
    in_file.close();
    std::cout << " and " << count << " valid trips." << std::endl;

    return count;
} // end readOneFileChicago


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         MAKEDCSTATIONMAP                           **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::makeDCStationMap ()
{
    const std::string dir = "data/"; 
    int ipos, tempi, count;
    bool tube;
    std::string fname;
    std::ifstream in_file;
    std::string linetxt, name;

    count = 0;

    fname = dir + "station_latlons_" + _city + ".txt";
    in_file.open (fname.c_str (), std::ifstream::in);
    assert (!in_file.fail ());

    in_file.clear ();
    in_file.seekg (0); 

    getline (in_file, linetxt, '\n'); // header

    while (getline (in_file, linetxt,'\n'))
    {
        for (int i=0; i<3; i++)
        {
            ipos = linetxt.find(',', 0);
            linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        }
        ipos = linetxt.find (',', 0);

        // Remove trailing spaces (for example from #31089
        name = linetxt.substr (0, ipos);
        name.erase (std::remove_if (name.end() - 1, name.end(), 
                    ::isspace), name.end());
        DCStationNameMap [name] = count;

        linetxt = linetxt.substr (ipos + 1, linetxt.length () - ipos - 1);
        DCStationNumberMap [linetxt] = count;
        count++;
    }
    in_file.close();

    return 0;
} // end makeDCStationMap



/************************************************************************
 ************************************************************************
 **                                                                    **
 **                           READONEFILEDC                            **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int RideData::readOneFileDC (int filei)
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
    int count = 0, ipos [2], tempi [4];
    int nstations = getStnIndxLen ();
    std::ifstream in_file;
    std::string linetxt, station;

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
    count = 0;

    /*
     * Unlike all others, the Washington DC trip files are inconsistently
     * formatted, and headers thus have to be scanned.
     */
    int startcol = INT_MIN, endcol = INT_MIN, starti, endi;
    std::string tempstr;
    getline (in_file, linetxt, '\n');
    while ((ipos [0] = linetxt.find (",")) != std::string::npos)
    {
        tempstr = linetxt.substr (0, ipos [0]).c_str ();
        std::transform (tempstr.begin(), tempstr.end (),
                tempstr.begin(), ::tolower);
        if (tempstr == "start station")
            startcol = count;
        else if (tempstr == "end station")
            endcol = count;

        linetxt = linetxt.substr (ipos [0] + 1, linetxt.length () - ipos [0] - 1);
        count++;
    }
    /* 
     * linetxt still has last entry at this point, but this is never start or
     * end station. Note that the following also presumes that endi > starti
     * *ALWAYS*!
     */
    assert (startcol >= 0);
    assert (endcol >= 0);

    /*
     * The DC data are truly messy, and alternative station names do not come
     * with lat/lon coordinates, so there's no simple way to re-map them.
     * Rather, those that don't match the "official" list simply have to be
     * manually substituted with the following values.
     */
    boost::unordered_map <std::string, std::string> nameSubs;
    typedef boost::unordered_map <std::string, std::string>::iterator
        nameSubsIterator;

    nameSubs ["Court House Metro / Wilson Blvd & N Uhle St"] =
            "Court House Metro / 15th & N Uhle St";
    nameSubs ["N Highland St & Wilson Blvd"] =
            "Clarendon Metro / Wilson Blvd & N Highland St";
    nameSubs ["Central Library"] = "Central Library / N Quincy St & 10th St N";
    nameSubs ["Randle Circle & Minnesota Ave NE"] =
            "Randle Circle & Minnesota Ave SE";
    nameSubs ["1st & N St SE"] = "1st & N St  SE";
    nameSubs ["N Fillmore St & Clarendon Blvd"] =
            "Clarendon Blvd & N Fillmore St";
    nameSubs ["26th & Crystal Dr"] = "27th & Crystal Dr"; // Just a guess
    nameSubs ["17th & K St NW [formerly 17th & L St NW]"] = "17th & K St NW";
    nameSubs ["4th St & Rhode Island Ave NE"] = "4th & W St NE";
    nameSubs ["18th & Hayes St"] = "Aurora Hills Community Ctr/18th & Hayes St";
    nameSubs ["12th & Hayes St"] = "Pentagon City Metro / 12th & S Hayes St";
    nameSubs ["15th & Hayes St"] = "Pentagon City Metro / 12th & S Hayes St";
    nameSubs ["Wisconsin Ave & Macomb St NW"] = "Wisconsin Ave & Newark St NW";
    nameSubs ["Court House Metro / Wilson Blvd & N Uhle St"] =
            "Court House Metro / 15th & N Uhle St";
    nameSubs ["Virginia Square"] =
            "Virginia Square Metro / N Monroe St & 9th St N";
    nameSubs ["23rd & Eads"] = "23rd & Eads St";
    nameSubs ["18th & Bell St"] = "Crystal City Metro / 18th & Bell St";
    nameSubs ["5th & K St NW"] = "5th St & K St NW";
    nameSubs ["4th St & Massachusetts Ave NW"] =
            "5th St & Massachusetts Ave NW";
    nameSubs ["16th & U St NW"] = "New Hampshire Ave & T St NW"; // No such station
    nameSubs ["7th & Water St SW / SW Waterfront"] =
            "6th & Water St SW / SW Waterfront";
    nameSubs ["McPherson Square - 14th & H St NW"] = "15th & K St NW";
    nameSubs ["5th St & K St NW"] = "5th & K St NW";
    nameSubs ["1st & N ST SE"] = "1st & N St  SE";
    nameSubs ["Fairfax Dr & Glebe Rd"] = "Glebe Rd & 11th St N"; // No such station
    nameSubs ["Idaho Ave & Newark St NW [on 2nd District patio]"] =
        "Wisconsin Ave & Newark St NW";
    nameSubs ["New Hampshire Ave & T St NW [formerly 16th & U St NW]"] =
        "New Hampshire Ave & T St NW";
    nameSubs ["8th & F St NW / National Portrait Gallery"] =
        "7th & F St NW / National Portrait Gallery";
    nameSubs ["Pentagon City Metro / 12th & Hayes St"] =
        "Pentagon City Metro / 12th & S Hayes St";
    nameSubs ["12th & Hayes St /  Pentagon City Metro"] =
        "Pentagon City Metro / 12th & S Hayes St";
    nameSubs ["Fallsgove Dr & W Montgomery Ave"] =
        "Fallsgrove Dr & W Montgomery Ave";
    nameSubs ["Bethesda Ave & Arlington Blvd"] = "Bethesda Ave & Arlington Rd";
    nameSubs ["Thomas Jefferson Cmty Ctr / 2nd St S & Ivy"] =
        "TJ Cmty Ctr / 2nd St & S Old Glebe Rd";
    nameSubs ["McPherson Square / 14th & H St NW"] = "15th & K St NW";
    nameSubs ["13th & U St NW"] = "12th & U St NW";
    nameSubs ["Connecticut Ave & Nebraska Ave NW"] =
        "Connecticut & Nebraska Ave NW";
    nameSubs ["18th & Wyoming Ave NW"] = "18th St & Wyoming Ave NW";
    nameSubs ["Connecticut Ave & Yuma St NW"] = "Van Ness Metro / UDC";
    nameSubs ["22nd & Eads St"] = "23rd & Eads St";

    count = 0;
    while (getline (in_file, linetxt,'\n')) {
        starti = endi = -INT_MAX;
        for (int i=0; i<startcol; i++) {
            ipos [0] = linetxt.find(",",0);
            linetxt = linetxt.substr (ipos [0] + 1, linetxt.length () - ipos [0] - 1);
        }
        ipos [0] = linetxt.find (",", 0);
        /*
         * The earlier files (prior to 2012) have station name followed by
         * parenthesised terminalName (which is a number). Since then, only
         * station names are present, yet these unfortunately change and require
         * individual replacements as stated below. Any additional differences
         * yet to come will be caught by the asserts!
         */
        ipos [1] = linetxt.find ("(", 0);
        if (ipos [1] != std::string::npos && ipos [1] < ipos [0])
        {
            linetxt = linetxt.substr (ipos [1] + 1, 
                    linetxt.length () - ipos [1] - 1);
            ipos [0] = linetxt.find (")", 0);
            station = linetxt.substr (0, ipos [0]);
            /*
             * There is one doubly-parenthesised station ("(Dupont Circle
             * South)"), with the number only coming in the second parentheses.
             */
            if (!std::all_of (station.begin(), station.end(), ::isdigit))
            {
                linetxt = linetxt.substr (ipos [0] + 1, 
                        linetxt.length () - ipos [0] - 1);
                ipos [0] = linetxt.find ("(", 0);
                assert (ipos [0] != std::string::npos);
                linetxt = linetxt.substr (ipos [0] + 1, 
                        linetxt.length () - ipos [0] - 1);
                ipos [0] = linetxt.find (")", 0);
                station = linetxt.substr (0, ipos [0]);
            }
            /*
             * 31999 == "Alta Bicycle Share Demonstation Station"
             * 31900 == "Birthday Station" (22 Sep 2011 only)
             * There is also "Alta Tech Office" which only appears in 2013-Q3 on
             * two self-trips
             */
            if (station != "31999" && station != "31900") 
            {
                if (DCStationNumberMap.find (station) == DCStationNumberMap.end())
                    std::cout << count<< ": station[" << station << 
                        "] start not found in number map!" << std::endl;
                assert (DCStationNumberMap.find (station) != 
                        DCStationNumberMap.end());
                starti = DCStationNumberMap [station];
            }
        }
        else
        {
            station = linetxt.substr (0, ipos [0]);
            station.erase (std::remove_if (station.end() - 1, station.end(), 
                        ::isspace), station.end());
            if (DCStationNameMap.find (station) == DCStationNameMap.end())
            {
                if (nameSubs.find (station) != nameSubs.end ())
                    station = nameSubs[station];
                /*
                for (nameSubsIterator itr = nameSubs.begin(); 
                        itr != nameSubs.end (); itr++)
                    if (station == itr->first)
                        station = itr->second;
                 */
            } 
            if (DCStationNameMap.find (station) != DCStationNameMap.end())
                starti = DCStationNameMap [station];
            else if (missingStationNames.find (station) ==
                    missingStationNames.end ())
                missingStationNames.insert (station);
        }

        // Then same for end station
        for (int i=0; i<(endcol - startcol); i++)
        {
            ipos [0] = linetxt.find (",", 0);
            linetxt = linetxt.substr (ipos [0] + 1, linetxt.length () - ipos [0] - 1);
        }
        ipos [0] = linetxt.find (",", 0);
        ipos [1] = linetxt.find ("(", 0);
        if (ipos [1] != std::string::npos && ipos [1] < ipos [0])
        {
            linetxt = linetxt.substr (ipos [1] + 1, 
                    linetxt.length () - ipos [1] - 1);
            ipos [0] = linetxt.find (")", 0);
            station = linetxt.substr (0, ipos [0]);
             // check again for doubly-parenthesised stations
            if (ipos [0] > 1 &&
                    !std::all_of (station.begin(), station.end(), ::isdigit))
            {
                linetxt = linetxt.substr (ipos [0] + 1, 
                        linetxt.length () - ipos [0] - 1);
                ipos [0] = linetxt.find ("(", 0);
                assert (ipos [0] != std::string::npos);
                linetxt = linetxt.substr (ipos [0] + 1, 
                        linetxt.length () - ipos [0] - 1);
                ipos [0] = linetxt.find (")", 0);
                station = linetxt.substr (0, ipos [0]);
            }
            if (station != "31999" && station != "31900" && ipos [0] > 1) 
            {
                if (DCStationNumberMap.find (station) == DCStationNumberMap.end())
                    std::cout << count<< ": station[" << station << 
                        "] end not found in number map!" << std::endl;
                assert (DCStationNumberMap.find (station) != 
                        DCStationNumberMap.end());
                endi = DCStationNumberMap [station];
            }
        }
        else if (ipos [0] > 1)
        {
            station = linetxt.substr (0, ipos [0]);
            station.erase (std::remove_if (station.end() - 1, station.end(), 
                        ::isspace), station.end());
            if (DCStationNameMap.find (station) == DCStationNameMap.end())
            {
                if (nameSubs.find (station) != nameSubs.end ())
                    station = nameSubs[station];
                /*
                for (nameSubsIterator itr = nameSubs.begin(); 
                        itr != nameSubs.end (); itr++)
                    if (station == itr->first)
                        station = itr->second;
                 */
            } 
            if (DCStationNameMap.find (station) != DCStationNameMap.end())
                endi = DCStationNameMap [station];
            else if (missingStationNames.find (station) ==
                    missingStationNames.end ())
                missingStationNames.insert (station);
        }

        // Note that unlike all other cities, DC does not use the _stationIndex!
        if (starti >= 0 && endi >= 0 && starti != endi)
        {
            ntrips (starti, endi) += 1.0;
            count++;
        }
    } // end while getline
    in_file.close();
    std::cout << " and " << count << " valid trips." << std::endl;

    return count;
} // end readOneFileDC



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

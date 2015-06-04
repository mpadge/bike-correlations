/***************************************************************************
 * This software is in the public domain, furnished "as is", without technical
 * support, and with no warranty, express or implied, as to its usefulness for
 * any purpose.
 *
 * <mainBikes.c++>
 *
 * Author: Mark Padgham, May 2015
 ***************************************************************************/

#include "mainBikes.h"

int main(int argc, char *argv[]) {
    int nfiles, count, tempi [2];
    std::vector <std::string> tempstr;
    std::string city, nfext, fname, r2name, covname, MIname;

    std::cout << std::endl << "_____________________________________________" << 
        "____________________________________________" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\t./bikes with the following 3 parameters " <<
        "(defaulting to first values):\t\t|" << std::endl;
    std::cout << "|\t1. <city> for <city>=<london/nyc/boston/chicago/" <<
        "washingtondc>\t\t\t|" << std::endl;
    std::cout << "|\t2. (0,1,2) for analyses of (all, subscriber, customer)" <<
        " data\t\t\t|" << std::endl;
    std::cout << "|\t\t\t\t(NYC/Boston/Chicago only)\t\t\t\t|" << std::endl;
    std::cout << "|\t ---or set the second parameter to >2 for analysis of" <<
        " age-class data---\t\t|" << std::endl;
    std::cout << "|\t ---in which case the 3rd parameter is the decade to"
        " be analysed---\t\t|" << std::endl;
    std::cout << "|\t ---or set to 0 for \"young\" = <40, and 1 for \"old\"" <<
        "---\t\t\t\t|" << std::endl;
    std::cout << "|\t3. (0,1,2) for analyses of (all, male, female) " <<
        "data (NYC/Boston/Chicago only)\t|" << std::endl;
    std::cout << "|\t(NOTE that (male,female) can only be analysed for " <<
        "subscribers.)\t\t\t|" << std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
    std::cout << "|\tCalculations loop over the two conditions of:\t\t\t\t\t|" <<
        std::endl;
    std::cout << "|\t1.(include, ignore) stations with zero trips\t\t\t\t\t|" << 
        std::endl;
    std::cout << "|\t2. analyses of (all, near, far) data\t\t\t\t\t\t|" << 
        std::endl;
    std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;

    city = "nyc";
    tempi [0] = tempi [1] = 0;
    count = -1;
    while (*++argv != NULL)
    {
        if (count < 0)
        {
            city = *argv;
            std::transform (city.begin(), city.end(), city.begin(), ::tolower);
            if (city.substr (0, 2) == "lo")
                city = "london";
            else if (city.substr (0, 3) == "oys")
                city = "oyster";
            else if (city.substr (0, 2) == "bo")
                city = "boston";
            else if (city.substr (0, 2) == "ch")
                city = "chicago";
            else if (city.substr (0, 2) == "wa" || city.substr (0, 2) == "dc")
                city = "washingtondc";
            else
                city = "nyc";
        } else {
            tempi [count] = atoi (*argv);
            if (tempi [count] < 0)
                tempi [count] = 0; 
        }
        count++;
    }
    RideData rideData (city, tempi [0], tempi [1]);

    std::cout << "|\t\tcity = " << city;
    if (city == "london" || city == "washingtondc")
        std::cout << "\t\t\t\t\t\t|" << std::endl;
    else
    {
        std::cout << " --- data = (";
        if (tempi [0] < 3)
        {
            tempstr.resize (0);
            tempstr.push_back ("all");
            tempstr.push_back ("subscriber");
            tempstr.push_back ("customer");
            tempstr.push_back ("age");
            std::cout << tempstr [tempi [0]] << ", ";
            tempstr.resize (0);
            tempstr.push_back ("all");
            tempstr.push_back ("male");
            tempstr.push_back ("female");
            std::cout << tempstr [tempi [1]] << ")";
            if (tempi [0] == 0 || (tempi [0] == 1 && tempi [1] == 0) ||
                    (tempi [0] == 2 && tempi [1] < 2))
                std::cout << "\t";
            std::cout << "\t\t\t\t|" << std::endl;
            tempstr.resize (0);
        }
        else
        {
            std::cout << "age)\t\t\t\t\t\t|" << std::endl;
        }
    }
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;

    int numStations = rideData.returnNumStations();
    std::cout << "There are " << numStations << 
        " stations [max#=" << rideData.getStnIndxLen() << "] and " << 
        rideData.getNumFiles() << " trip files." << std::endl;

    count = 0;
    if (city == "london") 
    {
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            nfiles = rideData.countFilesLondon (i);
            for (int j=0; j<nfiles; j++) {
                tempi [0] = rideData.unzipOneFileLondon (i, j);
                std::cout << "Reading file[";
                if (j < 10)
                    std::cout << " ";
                std::cout << j << "/" << nfiles << "]:" << rideData.fileName;
                std::cout.flush ();
                tempi [0] = rideData.readOneFileLondon ();
                std::cout << " = " << tempi [0] << " trips." << std::endl;
                count += tempi [0];
                tempi [0] = rideData.removeFile ();
            }
            rideData.dumpMissingStations ();
            std::cout << "Total Number of Trips = " << count << std::endl;
        } // end for i
        fname = "NumTrips_london.csv";
    } 
    else if (city == "nyc")
    {
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            tempi [0] = rideData.getZipFileNameNYC (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileNYC (i);
                tempi [0] = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
        rideData.summaryStats ();
        tempi [0] = rideData.aggregateTrips ();
        fname = "NumTrips_nyc_" + std::to_string (rideData.getSubscriber()) +
            std::to_string (rideData.getGender ()) + ".csv";
    } 
    else if (city == "boston")
    {
        for (int i=0; i<rideData.getNumFiles(); i++) // Only 1 file!
        {
            tempi [0] = rideData.getZipFileNameBoston (i);
            count = rideData.readOneFileBoston (i);
            tempi [0] = rideData.removeFile ();
        }
        std::cout << "Total number of trips = " << count << std::endl;
        rideData.summaryStats ();
        tempi [0] = rideData.aggregateTrips ();
        fname = "NumTrips_boston_" + std::to_string (rideData.getSubscriber()) +
            std::to_string (rideData.getGender ()) + ".csv";
    }
    else if (city == "chicago")
    {
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            tempi [0] = rideData.getZipFileNameChicago (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileChicago (i);
                tempi [0] = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
        rideData.summaryStats ();
        tempi [0] = rideData.aggregateTrips ();
        fname = "NumTrips_chicago_" + std::to_string (rideData.getSubscriber()) +
            std::to_string (rideData.getGender ()) + ".csv";
    }
    else if (city == "washingtondc")
    {
        std::ofstream out_file;
        tempi [0] = rideData.makeDCStationMap ();
        for (int i=0; i<rideData.getNumFiles(); i++)
        {
            tempi [0] = rideData.getZipFileNameNYC (i);
            if (rideData.fileName != "") {
                count += rideData.readOneFileDC (i);
                tempi [0] = rideData.removeFile ();
            }
        } // end for i
        std::cout << "Total number of trips = " << count << std::endl;
        if (rideData.missingStationNames.size () > 1)
        {
            // There is always "Alta Tech Office" - see note in RideData.c++
            std::cout << "The following stations are present in trip data " <<
                " yet not in station_latlons_washingtondc.txt:" << std::endl;
            for (boost::unordered_set <std::string>::iterator
                    itr = rideData.missingStationNames.begin ();
                    itr != rideData.missingStationNames.end (); itr++)
                std::cout << "\t[" << *itr << "]" << std::endl;
        }
        fname = "NumTrips_washingtondc.csv";
    }

    assert (rideData.readDMat () == 0);
    if (city != "boston" && city != "chicago" && city != "washingtondc")
        rideData.writeDMat (); 
    rideData.writeNumTrips (fname);

    // Then loop over (all, near, far) data
    std::string stdtxt;
    if (rideData.getStandardise ())
        stdtxt = "std_";
    else
        stdtxt = "unstd_";
    for (int i=0; i<3; i++)
    {
        rideData.nearfar = i;
        rideData.txtnf = rideData.txtnflist [i];

        if (city == "london" || city == "washingtondc")
        {
            r2name = "R2_" + city + "_from_" + rideData.txtnf + ".csv";
            covname = "Cov_" + city + "_from_" + stdtxt + rideData.txtnf + ".csv";
            MIname = "MI_" + city + "_from_" + rideData.txtnf + ".csv";
        }
        else if (city == "nyc" || city == "boston" || city == "chicago")
        {
            r2name = "R2_" + city + "_from_" + rideData.txtnf + "_" +
                std::to_string (rideData.getSubscriber ()) +
                std::to_string (rideData.getGender ()) + ".csv";
            covname = "Cov_" + city + "_from_" + stdtxt + rideData.txtnf + "_" +
                std::to_string (rideData.getSubscriber ()) +
                std::to_string (rideData.getGender ()) + ".csv";
            MIname = "MI_" + city + "_from_" + rideData.txtnf + "_" +
                std::to_string (rideData.getSubscriber ()) +
                std::to_string (rideData.getGender ()) + ".csv";
        }

        rideData.calcR2 (true);
        //rideData.writeR2Mat (r2name);
        rideData.writeCovMat (covname);

        rideData.calcR2 (false);
        count = r2name.find ("_from_");
        r2name.replace (count, 6, "_to_");
        count = covname.find ("_from_");
        covname.replace (count, 6, "_to_");
        //rideData.writeR2Mat (r2name);
        rideData.writeCovMat (covname);

        rideData.calcMIMat (true);
        rideData.writeMIMat (MIname);
        count = MIname.find ("_from_");
        MIname.replace (count, 6, "_to_");
        rideData.calcMIMat (false);
        rideData.writeMIMat (MIname);
    }
    //rideData.readR2Mat (false);
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;
}

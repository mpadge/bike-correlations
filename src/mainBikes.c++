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


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                         MAIN FUNCTION                              **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

int main(int argc, char *argv[]) {
    int nfiles, count, tempi [2], age;
    std::vector <std::string> tempstr;
    std::string city, subscriber, gender, r2name, covname, MIname;

    // a=(0,1,2) = (all, customer, subscriber), or 3 = distance deciles
    // b=(0,1,2) = (all, male, female), or if a=3, 
    // b=0=young, 1=old, otherwise decades

    try {
        boost::program_options::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "print version std::string")
            ("help", "produce help message")    
            ;

        boost::program_options::options_description config("Configuration");
        config.add_options()
            ("city,c", boost::program_options::value <std::string> 
                (&city)->default_value ("nyc"), "city")
            ("subscriber,s", boost::program_options::value <std::string> 
                (&subscriber)->default_value ("all"), 
                "subscriber (all, customer, subscriber)")
            ("gender,g", boost::program_options::value <std::string> 
                (&gender)->default_value ("all"), 
                 "gender (all, male, female)")
            ("age,a", boost::program_options::value <int> 
                 (&age)->default_value (-1),
                 "age (-1=all, 0=young, 1=old, otherwise decades)")
            ;

        // Not used here
        boost::program_options::options_description hidden("Hidden options");
        hidden.add_options()
            ("hidden-option", boost::program_options::value
                <std::vector<std::string> >(), "hidden option")
            ;

        boost::program_options::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        boost::program_options::options_description visible("Allowed options");
        visible.add(generic).add(config);

        boost::program_options::variables_map vm;
        store(boost::program_options::command_line_parser(argc, argv).
                options(cmdline_options).run(), vm);

        notify(vm);

        if (vm.count("help")) {
            std::cout << visible << std::endl;
            return 0;
        }

        if (vm.count("version")) {
            std::cout << "bike-correlations, version 1.0" << std::endl;
            return 0;
        }

    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }    


    std::transform (city.begin(), city.end(), city.begin(), ::tolower);
    std::cout << "---city = " << city << "---" << std::endl;
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
    std::string cityCaps = city;
    std::transform (cityCaps.begin(), cityCaps.end(), cityCaps.begin(), 
            ::toupper);
    std::cout << "city = " << cityCaps;

    std::transform (subscriber.begin(), subscriber.end(), subscriber.begin(), 
            ::tolower);
    if (subscriber.substr (0, 1) == "a")
    {
        subscriber = "all";
        tempi [0] = 0;
    } else if (subscriber.substr (0, 1) == "s") {
        subscriber = "subscriber";
        tempi [0] = 1;
    } else if (subscriber.substr (0, 1) == "c") {
        subscriber = "customer";
        tempi [0] = 2;
    }
    std::cout << "; subscriber = " << subscriber << " / " << tempi [0];

    std::transform (gender.begin(), gender.end(), gender.begin(), 
            ::tolower);
    if (gender.substr (0, 1) == "a")
    {
        gender = "all";
        tempi [1] = 0;
    } else if (gender.substr (0, 1) == "m") {
        gender = "male";
        tempi [1] = 1;
    } else if (gender.substr (0, 1) == "f") {
        gender = "female";
        tempi [1] = 2;
    }
    std::cout << "; gender = " << gender << " / " << tempi [1];

    if (age > -1)
    {
        tempi [0] = 3;
        tempi [1] = age;
        if (age == 0)
            std::cout << "; age = young";
        else if (age == 1)
            std::cout << "; age = old";
        else
            std::cout << "; age decade = " << age;
    }
    std::cout << std::endl;

    RideData rideData (city, tempi [0], tempi [1]);

    int numStations = rideData.returnNumStations();
    std::cout << "There are " << numStations << 
        " stations [max#=" << rideData.getStnIndxLen() << "] and " << 
        rideData.getNumFiles() << " trip files." << std::endl;

    count = 0;
    if (city == "london") 
        readLondon (rideData);
    else if (city == "nyc")
        readNYC (rideData);
    else if (city == "boston")
        readBoston (rideData);
    else if (city == "chicago")
        readChicago (rideData);
    else if (city == "washingtondc")
        readDC (rideData);

    assert (rideData.readDMat () == 0);
    if (city != "boston" && city != "chicago" && city != "washingtondc")
        rideData.writeDMat (); 

    // Then loop over (all, near, far) data
    std::string stdtxt;
    if (rideData.getStandardise ())
        stdtxt = "std_";
    else
        stdtxt = "unstd_";

    int nloops = 3;
    if (rideData.returnDeciles ())
    {
        rideData.writeDistDeciles ();
        nloops = 10;
    }

    for (int i=0; i<nloops; i++)
    {
        if (!rideData.returnDeciles ())
        {
            rideData.nearfar = i;
            rideData.txtnf = rideData.txtnflist [i];
        } else {
            rideData.nearfar = 10 + i;
            rideData.txtnf = "D" + std::to_string (i);
        }

        if (city == "london" || city == "washingtondc")
        {
            r2name = "R2_" + city + "_from_" + rideData.txtnf + ".csv";
            covname = "Cov_" + city + "_from_" + stdtxt + rideData.txtnf + ".csv";
            MIname = "MI_" + city + "_from_" + rideData.txtnf + ".csv";
        }
        else if (city == "nyc" || city == "boston" || city == "chicago")
        {
            r2name = "R2_" + city + "_from_" + rideData.txtnf;
            covname = "Cov_" + city + "_from_" + stdtxt + rideData.txtnf;
            MIname = "MI_" + city + "_from_" + rideData.txtnf;

            if (!rideData.returnDeciles ())
            {
                r2name = r2name + "_" + std::to_string (rideData.getSubscriber ()) +
                    std::to_string (rideData.getGender ());
                covname = covname + "_" + 
                    std::to_string (rideData.getSubscriber ()) +
                    std::to_string (rideData.getGender ());
                MIname = MIname + "_" +
                    std::to_string (rideData.getSubscriber ()) +
                    std::to_string (rideData.getGender ());
            }
            r2name = r2name + ".csv";
            covname = covname + ".csv";
            MIname = MIname + ".csv";
        }

        rideData.calcR2 (true);
        rideData.writeR2Mat (r2name);
        rideData.writeCovMat (covname);
        rideData.writeMIMat (MIname);

        rideData.calcR2 (false);
        count = r2name.find ("_from_");
        r2name.replace (count, 6, "_to_");
        count = covname.find ("_from_");
        covname.replace (count, 6, "_to_");
        count = MIname.find ("_from_");
        MIname.replace (count, 6, "_to_");
        rideData.writeR2Mat (r2name);
        rideData.writeCovMat (covname);
        rideData.writeMIMat (MIname);
    }
    //rideData.readR2Mat (false);
    std::cout << "_____________________________________________" << 
        "____________________________________________" << std::endl << std::endl;
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          READLONDON                                **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readLondon (RideData rideData)
{
    int tempi, nfiles, count = 0;

    for (int i=0; i<rideData.getNumFiles(); i++)
    {
        nfiles = rideData.countFilesLondon (i);
        for (int j=0; j<nfiles; j++) {
            tempi = rideData.unzipOneFileLondon (i, j);
            std::cout << "Reading file[";
            if (j < 10)
                std::cout << " ";
            std::cout << j << "/" << nfiles << "]:" << rideData.fileName;
            std::cout.flush ();
            tempi = rideData.readOneFileLondon ();
            std::cout << " = " << tempi << " trips." << std::endl;
            count += tempi;
            tempi = rideData.removeFile ();
        }
        rideData.dumpMissingStations ();
        std::cout << "Total Number of Trips = " << count << std::endl;
    } // end for i

    std::string fname = "NumTrips_london.csv";
    assert (rideData.readDMat () == 0);
    rideData.writeNumTrips (fname);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            READNYC                                 **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readNYC (RideData rideData)
{
    int tempi, count = 0;

    for (int i=0; i<rideData.getNumFiles(); i++)
    {
        tempi = rideData.getZipFileNameNYC (i);
        if (rideData.fileName != "") {
            count += rideData.readOneFileNYC (i);
            tempi = rideData.removeFile ();
        }
    } // end for i
    std::cout << "Total number of trips = " << count << std::endl;
    rideData.summaryStats ();
    tempi = rideData.aggregateTrips ();
    assert (rideData.readDMat () == 0);
    std::string fname = "NumTrips_nyc_" + std::to_string (rideData.getSubscriber()) +
        std::to_string (rideData.getGender ()) + ".csv";
    rideData.writeNumTrips (fname);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          READBOSTON                                **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readBoston (RideData rideData)
{
    int tempi, count = 0;

    for (int i=0; i<rideData.getNumFiles(); i++) // Only 1 file!
    {
        tempi = rideData.getZipFileNameBoston (i);
        count = rideData.readOneFileBoston (i);
        tempi = rideData.removeFile ();
    }
    std::cout << "Total number of trips = " << count << std::endl;
    rideData.summaryStats ();
    tempi = rideData.aggregateTrips ();
    assert (rideData.readDMat () == 0);
    std::string fname = "NumTrips_boston_" + std::to_string (rideData.getSubscriber()) +
        std::to_string (rideData.getGender ()) + ".csv";
    rideData.writeNumTrips (fname);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                          READCHICAGO                               **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readChicago (RideData rideData)
{
    int tempi, count = 0;

    for (int i=0; i<rideData.getNumFiles(); i++)
    {
        tempi = rideData.getZipFileNameChicago (i);
        if (rideData.fileName != "") {
            count += rideData.readOneFileChicago (i);
            tempi = rideData.removeFile ();
        }
    } // end for i
    std::cout << "Total number of trips = " << count << std::endl;
    rideData.summaryStats ();
    tempi = rideData.aggregateTrips ();
    assert (rideData.readDMat () == 0);
    std::string fname = "NumTrips_chicago_" + std::to_string (rideData.getSubscriber()) +
            std::to_string (rideData.getGender ()) + ".csv";
    rideData.writeNumTrips (fname);
}


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                            READDC                                  **
 **                                                                    **
 ************************************************************************
 ************************************************************************/

void readDC (RideData rideData)
{
    int tempi, count = 0;

    tempi = rideData.makeDCStationMap ();
    for (int i=0; i<rideData.getNumFiles(); i++)
    {
        tempi = rideData.getZipFileNameNYC (i);
        if (rideData.fileName != "") {
            count += rideData.readOneFileDC (i);
            tempi = rideData.removeFile ();
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
    assert (rideData.readDMat () == 0);
    std::string fname = "NumTrips_washingtondc.csv";
    rideData.writeNumTrips (fname);
}

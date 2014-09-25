#include <dirent.h>
#include <stdlib.h> // for EXIT_FAILURE
#include <string>
#include <fstream>

#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <vector>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!

void getDir (std::vector <std::string>* filelist);
int getNumStations ();
void readLatLons (dvec* lons, dvec* lats);
void getStationNames (std::vector <std::string>* names);
int readData (dmat* ntrips, std::string fname);
ivec tripNumRange (dmat* ntrips);

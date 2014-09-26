#ifndef UTILS_H
#define UTILS_H

#include "Utils.h"

#endif

#ifndef STRUCTS_H
#define STRUCTS_H

#include "Structures.h"

#endif


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
int readData (imat* ntrips, std::string fname);
ivec tripNumRange (imat* ntrips);
void writeR2mat (dmat *r2mat, bool from);

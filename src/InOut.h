#include "Utils.h"
#include "Structures.h"

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

#ifndef INOUT_H
#define INOUT_H

void getDir (std::vector <std::string>* filelist);
int getStationIndex (std::string city, intPair* index);
void readLatLons (dvec* lons, dvec* lats, intPair* stationIndex);
void getStationNames (std::vector <std::string>* names);
int readData (imat* ntrips, std::string fname);
ivec tripNumRange (imat* ntrips);
void writeR2mat (dmat *r2mat, bool from);

#endif

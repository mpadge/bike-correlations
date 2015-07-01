/*
 * Utils.h
 */

#include <stdlib.h> // has abs function
#include <math.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <vector>
#include <string>
#include <iomanip> // for setfill
#include <sys/ioctl.h> // for console width: Linux only!
#include <ctype.h>
#include <fstream>
#include <assert.h>

#include <boost/config.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#ifndef UTILS_H
#define UTILS_H

#define PI 3.1415926535897932384626433832795

typedef boost::numeric::ublas::vector <int> ivec;
typedef boost::numeric::ublas::matrix <int> imat;
typedef boost::numeric::ublas::vector <double> dvec;
typedef boost::numeric::ublas::matrix <double> dmat;
typedef boost::numeric::ublas::vector <bool> bvec;
typedef boost::numeric::ublas::matrix <bool> bmat;
typedef boost::numeric::ublas::zero_matrix <double> zmat_d;
typedef boost::numeric::ublas::zero_matrix <int> zmat_i;

const double DOUBLE_MAX = std::numeric_limits<double>::max (),
    DOUBLE_MIN = -DOUBLE_MAX,
    FLOAT_MAX = std::numeric_limits <float>::max ();

struct myTime{
    int hh, mm;
    float ss;	};

struct DistStruct{
    double dx, dy, d;	};


struct RegrResults {
    double r2, cov, slope, intercept, SS, tval;      };

std::string standardise (std::string sin);
std::string substituteNames (bool tube, std::string str);
double calc_angle (double x, double y);
DistStruct getdists (double xa, double ya, double xb, double yb);
DistStruct convert_distance (double dist, double midx, double midy);
void timeout (double tseconds);
RegrResults regression (std::vector <double> x, std::vector <double> y);
double calcMI (std::vector <double> x, std::vector <double> y);
void progLine (double progress);

#endif

/*
 * Calculations.cc
 */

#include "Calculations.h"


/************************************************************************
 ************************************************************************
 **                                                                    **
 **                        GETCORRELATIONS                             **
 **                                                                    **
 ************************************************************************
 ************************************************************************/


dvec getCorrelations (imat* ntrips, dmat* r2mat, bool from)
{
    double tempd;
    dvec ranges (2);
    ranges (0) = DOUBLE_MAX;
    ranges (1) = DOUBLE_MIN;
    RegrResults regr;
    std::vector <double> vecA, vecB;

    int nstations = (*ntrips).size1 ();

    for (int i=0; i<nstations; i++) {
        (*r2mat) (i, i) = (*r2mat) (i, i) = NAN;
    }
    for (int i=0; i<(nstations - 1); i++) {
        for (int j=(i + 1); j<nstations; j++) {
            (*r2mat) (i, j) = (*r2mat) (j, i) = NAN;
        }
    }

    std::cout << "Calculating correlations ..." << std::endl;
    int count = 0;
    for (int i=0; i<(nstations - 1); i++) {
        for (int j=(i + 1); j<nstations; j++) {
            vecA.resize (0);
            vecB.resize (0);
            for (int k=0; k<nstations; k++) {
                if (k != i && k != j) {
                    if (from) {
                        if ((*ntrips) (k, i) > 0 && (*ntrips) (k, j) > 0) {
                            vecA.push_back ((double) (*ntrips) (k, i));
                            vecB.push_back ((double) (*ntrips) (k, j));
                        }
                    } else {
                        if ((*ntrips) (i, k) > 0 && (*ntrips) (j, k) > 0) {
                            vecA.push_back ((double) (*ntrips) (i, k));
                            vecB.push_back ((double) (*ntrips) (j, k));
                        }
                    }
                } // end if k != i
            } // end for k
            if (vecA.size () > 2 && vecB.size () > 2) {
                regr = regression (vecA, vecB);
                if (!isnan (regr.r2)) {
                    count++;
                    (*r2mat) (i, j) = (*r2mat) (j, i) = regr.r2;
                    if (regr.r2 < ranges (0)) ranges (0) = regr.r2;
                    else if (regr.r2 > ranges (1)) ranges (1) = regr.r2;
                }
            }
        } // end for j
        tempd = (double) i / ((double) nstations - 1.0);
        progLine (tempd);
    } // end for i
    progLine (1.0);
    std::cout << std::endl << count;
    count = nstations * (nstations - 1) / 2;
    std::cout << " / " << count << " pair-wise comparisons calculated." <<
        std::endl;

    vecA.resize (0);
    vecB.resize (0);

    return ranges;
} // end function vectorAnalyses

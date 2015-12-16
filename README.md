bike-correlations
=================

Aggregates matrices of numbers of trips between all pairs of bicycle and rail
stations in various cities (bikes for London, NYC, Boston, Chicago, Washington
DC; rail for London), and uses these to calculate pair-wise covariance matrices.
Trip data can be downloaded from repositories for the respective cities.

Also calculates matrices of inter-station distances, which are routed with
urban-region OSM files obtained from [mapzen](https://mapzen.com/metro-extracts)
or [bbbike.org](http://download.bbbike.org/osm/). Route preferences are
internally fixed (in `mainStnDists.h`) for cycling using the same weightings as
[routino](http://routino.org).

Also calculates equivalent matrices for train data for London, using the
Oystercard data sample from http://www.tfl.gov.uk/info-for/open-data-users/

### build:
1. cd ./build  
2. cmake ..  
3. make


[![Build
Status](https://travis-ci.org/mpadge/bike-correlations.svg?branch=master)](https://travis-ci.org/mpadge/bike-correlations)

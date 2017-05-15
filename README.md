bike-correlations
=================

Calculates pair-wise correlation and covariance maatrices for numbers of trips
between all pairs of stations for the public hire bicycle systems of London,
NYC, Boston, Chicago, Washington DC, and Los Angeles. Trip data must first be
downloaded and converted to matrices of aggregate numbers of trips between all
pairs of stations using the `R` package
[`bikedata`](https://github.com/mpadge/bikedata).

Also calculates matrices of inter-station distances using
[OpenStreetMap](http://openstreetmap.org) data extracted with the `R` package
[`osmdata`](https://github.com/osmdatar/osmdata) and routed using the `R`
package [`osmprob`](https://github.com/osm-router/osmprob).

Also calculates equivalent matrices for train data for London, using the
Oystercard data sample from http://www.tfl.gov.uk/info-for/open-data-users/

### build:
1. cd ./build  
2. cmake ..  
3. make


[![Build
Status](https://travis-ci.org/mpadge/bike-correlations.svg?branch=master)](https://travis-ci.org/mpadge/bike-correlations)

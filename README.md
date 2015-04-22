bike-correlations
=================

Calculates matrices of both correlations and distances between all pairs of
bicycle and rail stations in various cities (bikes for London, NYC, Boston,
Chicago; rail for London).  Correlations are based on ride data downloaded from
repositories for the respective cities.

Distances are routed with urban-region OSM files obtained from
[mapzen](https://mapzen.com/metro-extracts) or
[bbbike.org](http://download.bbbike.org/osm/). Route preferences are internally
fixed for cycling using the same weightings as [routino](http://routino.org).

Train data for London use the Oystercard data sample from
http://www.tfl.gov.uk/info-for/open-data-users/

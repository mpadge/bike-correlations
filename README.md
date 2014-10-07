bike-correlations
=================

Calculates matrices of both correlations and distances between all pairs of
bicycle hire stations (London and NYC). Distances are routed with `routino`,
using the weightings for bicycle travel specified in
`/data/routino-profiles.xml`. Routing requires an OSM File which is then
converted with the routino `planetsplitter` command to make a bunch of `.mem`
files with a designated prefix (here, ``lo`` or ``ny``). If these files don't
exist, then planetsplitter is called by `getStDists.py`.

Note that splitting a .osm.bz2 does not work, even through it is supposed to with routino.

`getDists.py` reads the `station_latlons.txt` file and calculates all
inter-station distances. *This takes a really long time!* For example, London
has ~750 stations, and if each routing takes ~2s (actually slightly faster),
this amounts to 156 hours.

Correlations are based on ride data downloaded from repositories for the
respective cities.

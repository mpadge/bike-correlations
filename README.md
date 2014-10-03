bike-correlations
=================

Calculates matrices of both correlations and distances between all pairs of
bicycle hire stations (London and NYC). Distances are routed with `routino`,
using the weightings for bicycle travel specified in
`/data/routino-profiles.xml`. Routing requires an OSM File which is then
converted with the routino `planetsplitter` command thus:

`./path/to/routino/src/planetsplitter --prefix=lo --tagging='path/to/routino/xml/routino-tagging.xml' /path/to/osm-data/planet-location.osm`

Note that splitting a .osm.bz2 does not work, even through it is supposed to with routino.

`getDists.py` reads the `station_latlons.txt` file and calculates all
inter-station distances. *This takes a really long time!*

Correlations are based on ride data downloaded from repositories for the
respective cities.

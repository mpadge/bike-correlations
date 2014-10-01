bike-correlations
=================

Calculates correlation matrix between all pairs of bicycle hire stations (London and NYC).

inter-station distances
======================

These are calculated using routino.org. This in turn requires an OSM File which is then converted with the routino `planetsplitter` command thus:

`./path/to/routino/src/planetsplitter --prefix=lo --tagging='path/to/routino/xml/routino-tagging.xml' /path/to/osm-data/planet-location.osm`

Note that splitting a .osm.bz2 does not work, even through it is supposed to with routino.


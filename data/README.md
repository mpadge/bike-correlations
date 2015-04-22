data
=================

Tube and NR station data are scraped with `getRailDists.py`, which also dumps
station names and lat-lons for both tube and NR systems. Note that the tube data
are from [this link](http://www.doogal.co.uk/london_stations.php), which
includes 608 stations from both tube and NR systems. This list is reduced to
tube stations only by selecting only those present in `../data/London tube
lines.csv` which is also downloaded from the above link.

Location data for bicycle systems are all downloadable with the trip data.

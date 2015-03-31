data
=================

Tube and NR station data are scraped with `getRailDists.py`, which also dumps station names and lat-lons for both tube and NR systems. Note that
the tube data are from
[this link](http://www.doogal.co.uk/london_stations.php), which includes 608 stations from both tube and NR systems. This list is reduced to
tube stations only by selecting only those present in `../data/London tube lines.csv` which is also downloaded from the above link.

Note that the latter file has two errors, the first of which is that:

Piccadilly,Sudbury Hill,Rayners Lane

should be

Piccadilly,South Harrow,Rayners Lane
Piccadilly,Sudbury Hill,South Harrow

and the second is that:

Metropolitan,Northwood Hills,Moor Park

should be

Metropolitan,Northwood Hills,Northwood
Metropolitan,Northwood,Moor Park

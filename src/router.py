# Calculates distance between two lat-lon coordinates as routed using
# routino.org. The main work is preparing the appropriate OSM network and
# extracting all nodes for routing. The desired lat-lon coordinates then need to
# be matched onto existing nodes within the osm network
#
# At the moment, it's just a proof of principle using a pair of dummy
# coordinates. Now it just needs to be extended to calculate all distances
# between all pairs in station_latlons ...
import time, subprocess, os.path, glob
from bs4 import BeautifulSoup

def getBounds ():
    with open ('/data/data/bikes_london/planet-london.osm') as f:
        for line in f:
            if line.find ("bounds") > -1:
                lineout = line
                break
    soup = BeautifulSoup (lineout)
    bounds = soup.find ("bounds")
    lats = [bounds.attrs['minlat'], bounds.attrs['maxlat']]
    lons = [bounds.attrs['minlon'], bounds.attrs['maxlon']]
    return zip (lats, lons)

def getAllNodes ():
    # OSM files are potentially huge, so the short and easy way of parsing
    # either crashes or takes an enormously long time. This much faster way
    # directly scans the osm file and extracts only the nodes.
    start = time.time ()
    ids = []
    lats = []
    lons = []
    count = 0
    with open ('/data/data/bikes_london/planet-london.osm') as f:
        for line in f:
            if line.find ('node') > -1 and line.find ('lat=') > 1:
                id = line.split ("id=")[1].split ("lat=")[0]
                ids.append (int (id.strip("' ").strip('" ')))
                lat = line.split ("lat=")[1].split ("lon=")[0]
                lats.append (float (lat.strip("' ").strip('" ')))
                lon = line.split ("lon=")[1].split ("version=")[0]
                lons.append (float (lon.strip("' ").strip('" ')))
    end = time.time ()
    print len (ids), " nodes extracted in ", end - start, "s."
    return zip (ids, lats, lons)

def matchNodes (lon0, lat0):
    ni = -1
    mindiff = float("inf")
    count = 0
    for (id, lat, lon) in nodes:
        diff = abs (lat - lat0) + abs (lon - lon0)
        if diff < mindiff:
            ni = count
            mindiff = diff
        count = count + 1
    return nodes [ni]

# This routine produces "quickest.html" describing the route. This file is then
# analysed with getDist.py
def doRoute (lat1, lon1, lat2, lon2):
    node1 = matchNodes (lon1, lat1)
    node2 = matchNodes (lon2, lat2)
    # profiles copied from  ../xml/routino-profiles.xml, same for translations.
    # Directory in first arg has to be changed to appropriate routine dir
    #args = ["./../src/router", "--prefix=lo", "--transport=bicycle",\
    args = ["./../../routino-2.7.2/src/router", "--prefix=lo",\
            "--transport=bicycle", "--quickest",\
            "--profiles=../data/routino-profiles.xml",\
            "--translations=../data/routino-translations.xml",\
            "--lon1="+str (node1[2]), "--lat1="+str (node1[1]),\
            "--lon2="+str (node2[2]), "--lat2="+str (node2[1])]
    subprocess.Popen (args)

def getDist ():
    f = open ("quickest.html")
    page = f.read ()
    f.close ()
    from bs4 import BeautifulSoup
    soup = BeautifulSoup (page)
    table = soup.findAll ("table")[0]
    dists = soup.findAll (attrs={"class", "d"})
    dtot = 0
    for d in dists:
        di = d.find(text=True).encode('utf-8').strip()
        dtot = dtot + float (di.split ("km") [0])
    return dtot


nodes = getAllNodes ()
# Then match lat and lon to particular node:
lon1 = -0.20503
lat1 = 51.51057
lon2 = -0.10818
lat2 = 51.50475
bbox = getBounds ()
if lat1 < float (bbox[0][0]) or lat1 > float (bbox[1][0]) or\
        lon1 < float (bbox[0][1]) or lon1 > float (bbox[1][1]) or\
        lat2 < float (bbox[0][0]) or lat2 > float (bbox[1][0]) or\
        lon2 < float (bbox[0][1]) or lon2 > float (bbox[1][1]):
        print "ERROR: lat-lons are outside bbox of OSM file"
else:
    start = time.time ()
    doRoute (lat1, lon1, lat2, lon2)
    end = time.time ()
    print "routed in ", end - start, "s"
    
count = 0
fname = "quickest.html"
while not os.path.exists (fname):
    time.sleep (1)
    count = count + 1
    if count > 20:
        break

if os.path.isfile (fname):
    d = getDist ()
    print "total distance = ", d, "km"
    # clean up to ensure getDist uses righ quickest, because these files are
    # created for each doRoute, 
    for filename in glob.glob ("quickest*.*"):
        os.remove (filename)
else:
    raise valueError("%s isn't a file" %fname)



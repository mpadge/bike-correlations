# OSM files are potentially huge, so the short and easy way of parsing either
# crashes or takes an enormously long time. This much faster way directly scans
# the osm file and extracts only the nodes.
import time
import subprocess 
from bs4 import BeautifulSoup

def getAllNodes ():
    # First extract all nodes from London osm:
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

def doRoute (lat1, lon1, lat2, lon2):
    node1 = matchNodes (lon1, lat1)
    node2 = matchNodes (lon2, lat2)
    # profiles copied from  ../xml/routino-profiles.xml, same for translations.
    # Directory in first arg has to be changed to appropriate routine dir
    args = ["./../src/router", "--prefix=lo", "--transport=bicycle",\
            "--quickest", "--profiles=profiles.xml",\
            "--translations=translations.xml",\
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
start = time.time ()
doRoute (lat1, lon1, lat2, lon2)
end = time.time ()
print "routed in ", end - start, "s"



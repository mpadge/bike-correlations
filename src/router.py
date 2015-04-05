# Calculates distance between two lat-lon coordinates as routed using
# routino.org. The main work is preparing the appropriate OSM network and
# extracting all nodes for routing. The desired lat-lon coordinates then need to
# be matched onto existing nodes within the osm network
#
# At the moment, it's just a proof of principle using a pair of dummy
# coordinates. Now it just needs to be extended to calculate all distances
# between all pairs in station_latlons ...
import os, time, subprocess, math, sys
import numpy as np
from bs4 import BeautifulSoup

from progressbar import Bar, ETA, Percentage, \
        ProgressBar, ReverseBar, RotatingMarker


def checkPlanetSplitter (city="london"):
    # Run planetsplitter if .mem files don't exist for city. Also unzips OSM
    # file if still in .bz2 format
    # Note that a > chmod u+x planetsplitter is likely necessary
    files = os.listdir (".") # from /src
    if city.lower ()[0] == "l":
        city = "london"
        prfx = "lo"
    elif city.lower()[0] == "n":
        city = "nyc"
        prfx = "ny"
    else:
        city = "boston"
        prfx = "bo"
    # First unzip
    datadir = "/data/data/bikes/"
    dfiles = os.listdir (datadir)
    fcheck = any (f.find (city) > -1 and f.endswith(".osm") for f in dfiles)
    if not any (f.find(city) > -1 and f.endswith (".osm") for f in dfiles):
        bf = [f for f in dfiles if f.find (city) > -1 and f.endswith (".bz2")]
        if not bf:
            print "ERROR: %s.bz2 file does not exist to unzip"
            # TODO: exception handler
        else:
            bf = datadir + bf [0]
            args = ["bunzip2", bf]
            print "Unzipping planet-%s-.osm ... " % city
            subprocess.Popen (args)
    if not any (f.startswith(prfx) and f.endswith(".mem") for f in files):
        planetfile = datadir + "planet-" + city + ".osm"
        args = ["./../../routino-2.7.2/src/planetsplitter", "--prefix=" + prfx,\
                "--tagging=../../routino-2.7.2/xml/routino-tagging.xml",\
                planetfile]
        print "planet-%s.osm not yet split. Running planetsplitter..." % city
        subprocess.Popen (args)
    else:
        print "%s already split" % city

def getBounds (city="london"):
    wd = '/data/data/bikes/'
    fname = wd + 'planet-' + city + '.osm'
    with open (fname) as f:
        for line in f:
            if line.find ("bounds") > -1:
                lineout = line
                break
    soup = BeautifulSoup (lineout)
    bounds = soup.find ("bounds")
    lats = [bounds.attrs['minlat'], bounds.attrs['maxlat']]
    lons = [bounds.attrs['minlon'], bounds.attrs['maxlon']]
    return zip (lats, lons)

def getAllNodes (city="london"):
    # OSM files are potentially huge, so the short and easy way of parsing
    # either crashes or takes an enormously long time. This much faster way
    # directly scans the osm file and extracts only the nodes.
    start = time.time ()
    ids = []
    lats = []
    lons = []
    count = 0
    nodeMin = sys.maxint
    nodeMax = -sys.maxint
    wd = '/data/data/bikes/'
    fname = wd + 'planet-' + city + '.osm'
    if city != 'boston':
        with open (fname) as f:
            for line in f:
                if line.find ('node') > -1 and line.find ('lat=') > 1:
                    # Using Soup is easier but is orders of magnitude slower!
                    #soup = BeautifulSoup (line)
                    #node = soup.find ("node")
                    #ids.append (int (node.attrs ["id"]))
                    #lats.append (float (node.attrs ["lat"]))
                    #lons.append (float (node.attrs ["lon"]))
                    # So values are directly stripped instead. Note that this
                    # requires lines to be ordered (id,lat,lon,version).
                    id = line.split ("id=")[1].split ("lat=")[0]
                    ids.append (int (id.strip("' ").strip('" ')))
                    lat = line.split ("lat=")[1].split ("lon=")[0]
                    lats.append (float (lat.strip("' ").strip('" ')))
                    lon = line.split ("lon=")[1].split ("version=")[0]
                    lons.append (float (lon.strip("' ").strip('" ')))
    else:
        with open (fname) as f:
            for line in f:
                if line.find ('node') > -1 and line.find ('lat=') > 1:
                    id = line.split ("id=")[1].split ("version=")[0]
                    ids.append (int (id.strip("' ").strip('" ')))
                    lat = line.split ("lat=")[1].split ("lon=")[0]
                    lats.append (float (lat.strip("' ").strip('" ')))
                    if line.find ('/>') > -1:
                        lon = line.split ("lon=")[1].split ('/>\n')[0]
                    else:
                        lon = line.split ("lon=")[1].split ('>\n')[0]
                    lons.append (float (lon.strip("' ").strip('" ')))
    end = time.time ()
    print "%s nodes (#%s--%s) extracted in %ss." % (len (ids), min (ids),\
            max (ids), end - start)
    return zip (ids, lats, lons)

def dist (fromlon, fromlat, tolon, tolat):
    x = (tolon - fromlon) * math.pi / 180
    y = (tolat - fromlat) * math.pi / 180
    d = math.sin (y / 2) * math.sin (y / 2) +\
        math.cos (tolat * math.pi / 180) *\
        math.cos (fromlat * math.pi / 180) *\
        math.sin (x / 2) * math.sin (x / 2)
    d = 2 * math.atan2 (math.sqrt (d), math.sqrt (1 - d));
    return d * 6371.0

def npdist (lats, lons):
    # lats, lons are lists of length n, and return is an np.array of length n-1
    # with distances between consective points
    fromlat = np.array (lats [:-1])
    fromlon = np.array (lons [:-1])
    tolat = np.array (lats [1:])
    tolon = np.array (lons [1:])
    x = (tolon - fromlon) * math.pi / 180
    y = (tolat - fromlat) * math.pi / 180
    d = np.sin (y / 2) * np.sin (y / 2) +\
        np.cos (tolat * math.pi / 180) *\
        np.cos (fromlat * math.pi / 180) *\
        np.sin (x / 2) * np.sin (x / 2)
    d = 2 * np.arctan2 (np.sqrt (d), np.sqrt (1 - d));
    return d * 6371.0

def getAllWays (city="boston"):
    start = time.time ()
    wd = '/data/data/bikes/'
    fname = wd + 'planet-' + city + '.osm'
    nfrom = []
    nto = []
    dists = []
    start = time.time ()
    widgets = ['Progress: ', Percentage(), ' ', Bar(marker='-'),
            ' ', ETA()]
    pbar = ProgressBar(widgets=widgets, maxval=10000).start()
    with open (fname) as f:
        nlines = 0
        for line in f:
            nlines += 1
        f.seek (0)
        inway = False
        highway = False
        count = 0
        for line in f:
            if line.find ('<way') > -1:
                inway = True
                nd=[]
            elif line.find ('</way>') > -1:
                inway = False
                if highway and nd [0] != nd [-1]:
                    lats = []
                    lons = []
                    for n in nd:
                        ni = [i for i in nodes if i[0] == n][0]
                        lats.append (ni [1])
                        lons.append (ni [2])
                    dists.append (sum (npdist (lats, lons)))
                    highway = False
            elif inway:
                if line.find ('<nd') > -1:
                    node = line.split ("ref=")[1].split('/>')[0]
                    nd.append (int (node.strip ("' ").strip ('" ')))
                elif line.find ('k="highway"') > -1:
                    highway = True
            count += 1
            pbar.update(10000 * count / nlines)
    pbar.finish()
    end = time.time ()
    print "%s ways extracted in %s" % (len (dists), end - start)
    return zip (ids, lats, lons)

def matchNodes (lon0, lat0, nodes):
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
def doRoute (lat1, lon1, lat2, lon2, nodes, city="london"):
    node1 = matchNodes (lon1, lat1, nodes)
    node2 = matchNodes (lon2, lat2, nodes)
    # profiles copied from  ../xml/routino-profiles.xml, same for translations.
    # Directory in first arg has to be changed to appropriate routine dir
    args = ["./../../routino-2.7.2/src/router", "--prefix="+city[:2],\
            "--transport=bicycle", "--quickest",\
            "--profiles=../data/routino-profiles.xml",\
            "--translations=../data/routino-translations.xml",\
            "--lon1="+str (node1[2]), "--lat1="+str (node1[1]),\
            "--lon2="+str (node2[2]), "--lat2="+str (node2[1])]
    # Popen.wait comes with a warning that should *not* affect router output.
    subprocess.Popen (args).wait()

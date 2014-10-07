# Primary routine is writeDMat, which calculates routino distances and writes
# them successively to an indexed file. Each routing takes maybe 2s, and with
# 750 stations in London, this means 156 hours of calculation, or about 1 week.
# For this reason, the routine can be interrupted at any time, and will simply
# start again where it left off.
import time, os, glob, sys, getopt
import numpy
import router

def getDist (html):
    f = open (html)
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

def getLatLons (city="london"):
    ids = []
    lats = []
    lons = []
    if city.lower () [0] == "l":
        fname = "../data/station_latlons_london.txt"
    else:
        fname = "../data/station_latlons_nyc.txt"
    with open(fname,'r') as f:
        for line in f:
            ls=line.split (",")
            if ls[0].isdigit ():
                ids.append (int (ls [0]))
                lats.append (float (ls [1]))
                lons.append (float (ls [2]))
    return zip (ids, lats, lons)

def writeDMat (latlons, nodes, city="london"):
    n = len (latlons) # number of calcultions < n
    ntot = n * (n - 1) / 2 # number of comparisons
    # create index into upper triangle excluding diagonal
    indx = numpy.triu_indices (n, 1) 
    indx = zip (indx [0], indx [1])
    start = time.time ()
    t1 = 0
    count = 0
    if city.lower ()[0] == "l":
        fname = '../results/station_dists_london.txt'
        city = "london"
    else:
        fname = '../results/station_dists_nyc.txt'
        city = "nyc"
    f = open (fname, 'a+')
    for line in f:
        count += 1
    indx = indx [count:]
    startcount = count
    for i in indx:
        idi = latlons [i[0]] [0]
        lati = latlons [i[0]] [1]
        loni = latlons [i[0]] [2]
        idj = latlons [i[1]] [0]
        latj = latlons [i[1]] [1]
        lonj = latlons [i[1]] [2]
        router.doRoute (lati, loni, latj, lonj, nodes, city)
        # router writes "quickest.html" which is then analysed with getDist.
        # This writing necessitates first checking that the file has been
        # written:
        check = 0
        html = os.path.abspath ("quickest.html")
        while not os.path.exists (html):
            time.sleep (0.1)
            check += 1
            if check > 20:
                break
        # ... and then making sure that writing has finished and the file
        # connection has been closed:
        check = 0
        while html in openFiles ():
            time.sleep (0.1)
            check += 1
            if check > 20:
                break
        if os.path.isfile (html):
            d = getDist (html)
            f.write (str (idi) + ", " + str (idj) + ", " + str (d) + '\n')
            f.flush ()
        # Then delete "quickest*.*:
        for filename in glob.glob ("quickest*.*"):
            os.remove (filename)
        # routino dumps waypoints to scrn, so progress is text-based
        telapsed = time.time () - start
        if (count - startcount) > 0:
            t1 = telapsed / (count - startcount)
        tremaining = t1 * (ntot - count)
        ls = ["-------- Calculated ", str (count), "/", str (ntot), " = ",\
            str (100 * count / ntot), "%; time [elapsed, remaining] = [",\
            tout (telapsed), ", ", tout (tremaining), "] --------\n"]
        print "".join (ls)
        count += 1
    end = time.time ()
    print "Inter-station routing finished in ", end - start, "s"
    f.close ()

def openFiles ():
    # Usage: if f in openFiles():, where f must be abspath
    ref={}
    for p in os.listdir("/proc/"):
        if not p.isdigit(): continue
        d = "/proc/%s/fd/" % p
        try:
            for fd in os.listdir(d):
                f = os.readlink(d+fd)
                if f not in ref: ref[f] = []
                ref[f].append(p)
        except OSError:
            pass
    return ref

def tout (t):
    # Overdone time formatter to put all appropriate zeros in place
    t = int (round (t))
    hh, rem = divmod (t, 3600)
    mm, ss = divmod (rem, 60)
    lst = []
    if t < 10:
        ls = ["0:00:0", str (t)]
    elif t < 60:
        ls = ["0:00:", str (t)]
    elif t<3600:
        if mm < 10 and ss < 10:
            ls = ["0:0", str (mm), ":0", str (ss)]
        elif mm < 10:
            ls = ["0:0", str (mm), ":", str (ss)]
        elif ss < 10:
            ls = ["0:", str (mm), ":0", str (ss)]
        else:
            ls = ["0:", str (mm), ":", str (ss)]
    else:
        if mm < 10 and ss < 10:
            ls = [str (hh), ":0", str (mm), ":0", str (ss)]
        elif mm < 10:
            ls = [str (hh), ":0", str (mm), ":", str (ss)]
        elif ss < 10:
            ls = [str (hh), ":", str (mm), ":0", str (ss)]
        else:
            ls = [str (hh), ":", str (mm), ":", str (ss)]
    return "".join (ls)

if __name__ == "__main__":
    opts, args = getopt.getopt (sys.argv[1:],[])
    if len (args) < 1:
        print "usage: getStDists <city=london/nyc>, defaulting to London"
        city = "london"
    else:
        if args [0].lower ()[0] == "l":
            city = "london"
        else:
            city = "nyc"

    print "Extracting OSM nodes ... "
    nodes = router.getAllNodes (city)
    bbox = router.getBounds (city)
    latlons = getLatLons (city)
    # Check that latlons are within OSM bbox:
    latmin = min (latlons) [1]
    latmax = max (latlons) [1]
    lonmin = min (latlons) [2]
    lonmax = max (latlons) [2]
    if latmin < float (bbox[0][0]) or latmax > float (bbox[1][0]) or\
            lonmin < float (bbox[0][1]) or lonmax > float (bbox[1][1]):
            print "ERROR: lat-lons are outside bbox of OSM file"
            sys.exit (0)
    else:
        writeDMat (latlons, nodes, city)

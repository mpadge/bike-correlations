import time, os.path, glob, sys
import numpy
import router

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

def getLatLons ():
    ids = []
    lats = []
    lons = []
    with open("../data/station_latlons.txt",'r') as f:
        for line in f:
            ls=line.split (",")
            if ls[0].isdigit ():
                ids.append (int (ls [0]))
                lats.append (float (ls [1]))
                lons.append (float (ls [2]))
    return zip (ids, lats, lons)

def getDMat (latlons, nodes):
    n = max (latlons) [0] # size of array
    nll = len (latlons) # number of calcultions < n
    np = nll * (nll - 1) / 2 # number of comparisons
    dmat = numpy.zeros ((n, n))
    start = time.time ()
    # Direct indexing is easier here than dmat.flat
    for i in range (nll-1):
        for j in range (nll-1):
            router.doRoute (latlons [i][1], latlons [i][2],\
                    latlons [j+1][1], latlons [j+1][2], nodes)
            # Wait for routino to write "quickest.html":
            count = 0
            fname = "quickest.html"
            while not os.path.exists (fname):
                time.sleep (1)
                count = count + 1
                if count > 20:
                    break
            if os.file.isfile (fname):
                # Note that station IDs for London are 1-indexed.
                ii = latlons [i][0] - 1
                ij = latlons [j+1][0] - 1
                dmat [ii][ij] = dmat [ij][ii] = getDist ()
            # Then delete "quickest*.*:
            for filename in glob.glob ("quickest*.*"):
                os.remove (filename)
            # routino dumps waypoints to scrn, so progress is text-based
            progress = (i * (n - 1) + j)
            print "-------- Calculated ", progress, " / ", np, " = ",\
                100 * progress / np, "% --------"
    end = time.time ()
    print "Inter-station routing finished in ", end - start, "s"
    return dmat

def writeDMat (dmat):
    n = dmat.shape [0]
    f = open ("../data/station_dists.txt", 'w')
    for i in range (n):
        for j in range (n-1):
            f.write (str (dmat [i][j]) + ',')
        f.write (str (dmat [i][n-1]) + '\n')
    f.close ()
    print "distmat written to station_dists.txt"

if __name__ == "__main__":
    print "Extracting OSM nodes ... "
    nodes = router.getAllNodes ()
    bbox = router.getBounds ()
    latlons = getLatLons ()
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
        dmat = getDMat (latlons, nodes)
        writeDMat (dmat)

from bs4 import BeautifulSoup

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


d = getDist ()
print "total distance = ", d, "km"

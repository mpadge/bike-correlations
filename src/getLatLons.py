#!/usr/bin/python

# Extracts lat-lons for all london hire bicycle stations.  Importing https in
# python requires ssl and is thus fraught with OS system differences and
# unreliable routines in urllib. To circumvent problems, this routine can be run
# by simply downloading the source of the webpage at
# https://web.barclayscyclehire.tfl.gov.uk/maps/
# which should be titled "maps.htm". The python script then extracts the lats
# and lons of all stations from this html source.
#
# The coordinates for citibikenyc are at
# http://www.citibikenyc.com/stations/json/

import sys, getopt, re, urllib2, json
from bs4 import BeautifulSoup

def getLondon ():
    f = open ('../data/maps.htm', 'r')
    page = f.read ()
    f.close ()
    soup = BeautifulSoup (page)
    list = soup.findAll ('script')
    list = [i for i in list if i.find (text=True)]
    list = [i for i in list if
            i.find(text=True).encode('utf-8').find('stationMarker') >= 0] [0]
    # list is a single bs4 object, which is then converted to a string
    list = list.find (text=True).encode ('utf-8') 
    list = list.split ('station=')
    fname = '../data/station_latlons_london.txt'
    f = open (fname, 'w')
    outs = ('id', 'lat', 'long', 'name')
    f.write ('id, lat, long, name\n')
    count = 0
    minstn = 9999
    maxstn = 0
    for li in list:
        if li.find ("name") > 0:
            # This regex extracts all text NOT contained within double quotes, which
            # thus contains all field names of the javascript source
            linames = re.findall ('(?:^|")([^"]*)(?:$|")', li)
            linames = [filter (None, re.findall ('([^{,:]*)', i))[0] for i in linames]
            # And this regex extract all text within double quotes, which are the
            # corresonding field values.
            lidat = re.findall ('"([^"]*)"', li)
            # f is written as comma-delimited, so remove commas from station names
            lidat = [i.replace (',', ' ') for i in lidat]
            indx = [linames.index (i) for i in outs]
            if len (indx) == len (outs):
                for i in indx:
                    f.write (lidat [i])
                    if i != indx [-1]:
                        f.write (',')

                count = count + 1
                f.write ('\n')
                # Update max & min station numbers
                stnum = int (lidat [linames.index ('id')])
                if stnum < minstn:
                    minstn = stnum
                elif stnum > maxstn:
                    maxstn = stnum

    f.close ()
    print count, 'London stations in [', minstn, ',', maxstn,
    print '] written to ', fname

def getNYC ():
    url = 'http://www.citibikenyc.com/stations/json/'
    content = urllib2.urlopen (url).read ()
    data = json.loads (content)
    data = data['stationBeanList']
    fname = '../data/station_latlons_nyc.txt'
    f = open (fname, 'w')
    f.write ('id, lat, long, name\n')
    count = 0
    minstn = 9999
    maxstn = 0
    for st in data:
        f.write (str (st['id']) + ", " + str (st['latitude']) + ", ")
        f.write (str (st['longitude']) + ", " + str (st['stationName']) + '\n')
        id = int (st['id'])
        if id < minstn: minstn = id
        elif id > maxstn: maxstn = id
        count += 1
    print count, 'NYC stations in [', minstn, ',', maxstn,
    print '] written to ', fname
    f.close ()


if __name__ == "__main__":
    opts, args = getopt.getopt (sys.argv[1:],[])
    if len (args) == 0:
        print "usage: getLatLons <city>=<london,nyc>; defaulting to NYC"
        city = 'london'
    else:
        if args [0].find ('l') > -1 or args [0].find('L') > -1:
            city = 'london'
        else:
            city = 'nyc'
    
    if city == 'london':
        getLondon ()
    else:
        getNYC ()

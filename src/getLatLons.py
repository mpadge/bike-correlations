#!/usr/bin/python

# Extracts lat-lons for all hire bicycle stations.  The coordinates for
# citibikenyc are automatically downloaded from
# http://www.citibikenyc.com/stations/json/
#
# London is more complicated for 2 reasons. First, the webpage is https, and
# importing in python requires ssl and is thus fraught with OS system
# differences and unreliable routines in urllib. To circumvent problems, this
# script has to be run on the html source downloaded from
# https://web.barclayscyclehire.tfl.gov.uk/maps/
# which should be titled "maps.htm". 
#
# A second problem is that the tfl page only lists currently operating stations,
# and so changes regularly, and will generally not included all historical
# stations. (Currently closed stations are listed at
# http://www.tfl.gov.uk/modes/cycling/barclays-cycle-hire/find-a-docking-station)
# This script can thus be run multiple times, with each newly appearing station
# appended to any pre-existing ones.

import sys, getopt, re, urllib2, json, os
from bs4 import BeautifulSoup
import pandas as pd

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
    outs = ('id', 'lat', 'long', 'name')
    # Values from maps.htm are appended to any existing values, and stripped
    # back to unique values with .drop_duplicates() afterwards
    if os.path.lexists (fname): 
        df = pd.read_csv (fname)
        df.columns = outs # necessary because to_csv inserts spaces
    else:
        df = pd.DataFrame(columns=outs)
    count = 0
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
                lidat = [num (lidat [i]) for i in indx]
                df = pd.concat ([df, pd.DataFrame (lidat, index=outs).transpose()])
                count = count + 1

    df = df.drop_duplicates ()
    df.to_csv (fname, index=False)
    ids = df["id"].convert_objects(convert_numeric=True)
    print "%s London stations in [%s - %s] written to %s" % (count, ids.min (),
        ids.max (), fname)

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

def num (s):
    try:
        return int (s)
    except ValueError:
        try:
            return float (s)
        except ValueError:
            return s
    except ValueError:
        return s

if __name__ == "__main__":
    opts, args = getopt.getopt (sys.argv[1:],[])
    if len (args) == 0:
        print "usage: getLatLons <city>=<london,nyc>; defaulting to London"
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

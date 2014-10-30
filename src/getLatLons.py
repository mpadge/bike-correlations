#!/usr/bin/python

# Extracts lat-lons for all hire bicycle stations.  Both London and NYC only
# list currently operating stations, so station lists change regularly, and each
# will generally not included all historical stations. (Currently closed
# stations in London are listed at
# http://www.tfl.gov.uk/modes/cycling/barclays-cycle-hire/find-a-docking-station)
# This script can thus be run multiple times, with each newly appearing station
# appended to any pre-existing ones.
#
# The coordinates for citibikenyc are automatically downloaded from
# http://www.citibikenyc.com/stations/json/
#
# London is more complicated because the webpage is https, and importing in
# python requires ssl and is thus fraught with OS system differences and
# unreliable routines in urllib. To circumvent problems, this script has to be
# run on the html source downloaded from
# https://web.barclayscyclehire.tfl.gov.uk/maps/ which should be titled
# "maps.htm". 

import sys, getopt, re, urllib2, json, os
from bs4 import BeautifulSoup
import pandas as pd

def rootDir ():
        return os.getcwd ().split ('/src')[0]

def getLondon ():
    f = open (rootDir() + '/data/maps.htm')
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
    outs = ('id', 'lat', 'long', 'name')
    # Values from maps.htm are appended to any existing values, and stripped
    # back to unique values with .drop_duplicates() afterwards
    fname = rootDir() + '/data/station_latlons_london.txt'
    if os.path.lexists (fname): 
        df = pd.read_csv (fname)
        df.columns = outs # necessary because to_csv inserts spaces
    else:
        df = pd.DataFrame(columns=outs)
    count = 0
    for li in list:
        if li.find ('name') > 0:
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

    df = df.drop_duplicates (cols=['id','name']).sort('id')
    df.to_csv (fname, index=False)
    ids = df['id'].convert_objects(convert_numeric=True)
    print '%s London stations in [%s - %s] written to %s' % (count, ids.min (),
            ids.max (), fname)

def getNYC ():
    url = 'http://www.citibikenyc.com/stations/json/'
    content = urllib2.urlopen (url).read ()
    data = json.loads (content)
    data = data['stationBeanList']
    outs = ('id', 'lat', 'long', 'name')
    fname = rootDir() + '/data/station_latlons_nyc.txt'
    if os.path.lexists (fname): 
        df = pd.read_csv (fname)
        df.columns = outs # necessary because to_csv inserts spaces
    else:
        df = pd.DataFrame(columns=outs)
    for st in data:
        st1 = [st['id'], st['latitude'], st['longitude'], st['stationName']]
        df = pd.concat ([df, pd.DataFrame (st1, index=outs).transpose()])
    
    df = df.drop_duplicates (cols='id').sort('id')
    df.to_csv (fname, index=False)
    ids = df['id'].convert_objects(convert_numeric=True)
    print '%s NYC stations in [%s - %s] written to %s' % (df.count()[0], ids.min
            (), ids.max (), fname)

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

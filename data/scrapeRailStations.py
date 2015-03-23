#!/usr/bin/python

from bs4 import BeautifulSoup
import os, sys, urllib2, time, string
import pandas as pd

url_data = "http://en.wikipedia.org/wiki/List_of_London_railway_stations"
content = urllib2.urlopen (url_data).read ()
soup = BeautifulSoup (content)
table = soup.find ("table")
rows = table.findAll ('tr')[1:] # rows[0] is the header

fmain = open ("London-rail-stations.txt", 'w')

count = 0
for ri in rows:
    cols = ri.findAll ('td')
    name = cols [0].find(text=True).encode('utf-8').strip()
    lat = cols [-1].findAll("span", {"class": "latitude"})
    lat = lat [0].find (text=True).encode ('utf-8')
    deg = lat.split ('\xc2')[0]
    min = lat.split ('\xb0')[1].split ('\xe2')[0]
    sec = lat.split ('\xb2')[1].split ('\xe2')[0]
    lat = float (deg) + float (min) / 60.0 + float (sec) / 3600.0
    lon = cols[-1].findAll("span", {"class": "longitude"})
    lon = lon [0].find (text=True).encode ('utf-8')
    deg = lon.split ('\xc2')[0]
    min = lon.split ('\xb0')[1].split ('\xe2')[0]
    sec = lon.split ('\xb2')[1].split ('\xe2')[0]
    ew = lon.split ('\xb3')[1]
    lon = float (deg) + float (min) / 60.0 + float (sec) / 3600.0
    if (ew == 'W'):
        lon = -lon
    count += 1
    if (name != 'Stratford International'):
        fmain.write (name + ", " + str (lat) + ", " + str (lon) + "\n")

print "%s stations written to London-rail.stations.txt" % count

fmain.close ()

# ----------- then strip london-tube-stations.txt --------------
import fileinput

fname = "London-tube-stations.txt"
flines = []
for line in open (fname).readlines ():
    lines = line.split (",")
    if lines[1].isdigit(): # False for header only
        flines.append (lines [0] + "," + lines[3] + "," + lines[4] + "\n")

f = open (fname, "w")
for line in flines:
    f.write (line)

f.close ()

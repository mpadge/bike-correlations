from __future__ import division # for <python3
import os, math, sys
import pandas as pd
import numpy as np

class LondonRailStations (object):
    def __init__(self, latlon_file, network_file):
        self.latlons = pd.read_csv (latlon_file, header=None,
                names=["name","lat","lon"])
        self.lines = pd.read_csv (network_file, header=None,
                names=["from","to"])
    def getDists (self):
        n = len (data.lines)
        self.lines = pd.DataFrame ({'from': self.lines['from'],
                                    'to': self.lines['to'],
                                    'd': pd.Series (None,range(n))})
        for index, row in self.lines.iterrows():
            st = row["from"]
            fromlon = float (self.latlons[self.latlons["name"] == st]["lon"])
            fromlat = float (self.latlons[self.latlons["name"] == st]["lat"])
            st = row["to"]
            tolon = float (self.latlons[self.latlons["name"] == st]["lon"])
            tolat = float (self.latlons[self.latlons["name"] == st]["lat"])
            x = (tolon - fromlon) * math.pi / 180
            y = (tolat - fromlat) * math.pi / 180
            d = math.sin (y / 2) * math.sin (y / 2) +\
                math.cos (tolat * math.pi / 180) *\
                math.cos (fromlat * math.pi / 180) *\
                math.sin (x / 2) * math.sin (x / 2)
            d = 2 * math.atan2 (math.sqrt (d), math.sqrt (1 - d));
            self.lines.loc [index, 'd'] = d * 6371

if __name__ == "__main__":
    latlon_file = "../data/London-rail-stations.txt"
    network_file = "../data/London-rail-lines.txt"
    data = LondonRailStations (latlon_file, network_file)
    print "There are %s stations" % len (data.latlons)
    print "And the network has %s entries" % len (data.lines)
    data.getDists ()
    data.lines = data.lines [['from', 'to', 'd']]
    data.lines.to_csv(network_file,header=False,index=False)
    print "Distances added to London-rail-lines.txt"


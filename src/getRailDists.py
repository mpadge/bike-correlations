from __future__ import division # for <python4
import os, math, sys, csv, urllib2, string, pqdict
import pandas as pd
import numpy as np
from bs4 import BeautifulSoup

def normalise(s):
    s.replace(' and ',' & ')
    s.replace(' And ',' & ')
    for p in string.punctuation:
        s = s.replace(p, '')
    return s.lower().strip()

class LondonRailStations (object):

    def __init__(self, tube=True):

        if tube:

            latlon_file = 'http://www.doogal.co.uk/LondonStationsCSV.php'
            content = urllib2.urlopen (latlon_file).read ()
            soup = BeautifulSoup (content)
            lines = soup.get_text().encode('utf-8').split('\n')
            lines = lines [1:(len (lines) - 1)]
            self.latlons = pd.DataFrame()

            for line in lines:
                ls = line.split (",")
                name = ls[0]
                lat = float (ls[3])
                lon = float (ls[4])
                dat = {'name':name, 'lat':lat, 'lon':lon}
                self.latlons = self.latlons.append (dat, ignore_index=True)

            network_file = '../data/London tube lines.csv'
            self.lines = pd.read_csv (network_file, header=0,
                    names=['line','from','to'],
                    usecols=['from','to'])
        else:

            latlon_file = "http://en.wikipedia.org/wiki/List_of_London_railway_stations"
            content = urllib2.urlopen (latlon_file).read ()
            soup = BeautifulSoup (content)
            table = soup.find ("table")
            rows = table.findAll ('tr')[1:] # rows[0] is the header
            self.latlons = pd.DataFrame()

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
                if (name != 'Stratford International'):
                    dat = {'name':name, 'lat':lat, 'lon':lon}
                    self.latlons = self.latlons.append (dat, ignore_index=True)

            network_file = "../data/London-rail-lines.txt"
            self.lines = pd.read_csv (network_file, header=None,
                    names=["from","to"])

        for index, row in self.latlons.iterrows ():
            self.latlons['name'][index] = normalise (row ['name'])
        for index, row in self.lines.iterrows ():
            self.lines['from'][index] = normalise (row ['from'])
            self.lines['to'][index] = normalise (row ['to'])

        if tube:
            # These latlons include all stations, so the self.latlons df is made
            # by appending only actual tube stations

            latlons = pd.DataFrame()
            for index, row in self.latlons.iterrows():
                found = [i for i in self.lines['from'] if i == row['name']]
                found += [i for i in self.lines['to'] if i == row['name']]
                if len (found) > 0:
                    dat = {'name':row['name'],'lat':row['lat'],'lon':row['lon']}
                    latlons = latlons.append (dat, ignore_index=True)
            self.latlons = latlons

    def getDists (self):

        n = len (self.lines)
        self.lines = pd.DataFrame ({'from': self.lines['from'],
                                    'to': self.lines['to'],
                                    'd': pd.Series (None,range(n))})

        for index, row in self.lines.iterrows():
            st = row["from"]
            fromlon = float (self.latlons [self.latlons["name"] == st]["lon"])
            fromlat = float (self.latlons [self.latlons["name"] == st]["lat"])
            st = row["to"]
            tolon = float (self.latlons [self.latlons["name"] == st]["lon"])
            tolat = float (self.latlons [self.latlons["name"] == st]["lat"])
            x = (tolon - fromlon) * math.pi / 180
            y = (tolat - fromlat) * math.pi / 180
            d = math.sin (y / 2) * math.sin (y / 2) +\
                math.cos (tolat * math.pi / 180) *\
                math.cos (fromlat * math.pi / 180) *\
                math.sin (x / 2) * math.sin (x / 2)
            d = 2 * math.atan2 (math.sqrt (d), math.sqrt (1 - d));
            self.lines.loc [index, 'd'] = d * 6371

    def makeGraph (self): 

        if not hasattr (self.lines, 'd'):
            self.getDists ()
        self.graph = {}

        for index, row in self.lines.iterrows():
            stfrom = row['from']
            stto = row['to']
            if not stfrom in self.graph:
                self.graph [stfrom] = {row['to']: row['d']}
            elif not stto in self.graph [stfrom]:
                self.graph [stfrom] [stto] = row['d']
            if not stto in self.graph:
                self.graph [stto] = {row['from']: row['d']}
            elif not stfrom in self.graph [stto]:
                self.graph [stto] [stfrom] = row['d']

    '''
    The graph data then have to converted to a distance matrix (using dijkstra),
    and written *IN THE STATION ORDER GIVEN IN SELF.LATLONS*
    '''

    def makeDMat (self): 

        if not hasattr (self, 'graph'):
            self.makeGraph ()
        n = len (self.latlons)
        self.dmat = np.zeros ((n, n))

        for index, row in self.latlons.iterrows():
            dist, pred = dijkstra (self.graph, source = row['name'])
            vals = [value for key,value in sorted (dist.items())]
            self.dmat [index,] = vals

        self.dmatNames = sorted (dist.keys())


def dijkstra(graph, source, target=None):
    '''
    SP from one source to all other vertices, from:
    https://github.com/nvictus/priority-queue-dictionary/
        blob/master/examples/dijkstra.py
    Uses priority queues, so is O( (m+n) log n ), where n is the number of
    vertices and m is the number of edges. If the graph is connected
    (i.e. the graph is in one piece), m normally dominates over n, making the
    algorithm O(m log n) overall.
    '''
    dist = {}   
    pred = {}
    # Store distance scores in a priority queue dictionary
    pq = pqdict.PQDict()
    for node in graph:
        if node == source:
            pq[node] = 0
        else:
            pq[node] = float('inf')
    # Remove the head node of the "frontier" edge from pqdict: O(log n).
    for node, min_dist in pq.iteritems():
        # Each node in the graph gets processed just once.
        # Overall this is O(n log n).
        dist[node] = min_dist
        if node == target:
            break
        # Updating the score of any edge's node is O(log n) using pqdict.
        # There is _at most_ one score update for each _edge_ in the graph.
        # Overall this is O(m log n).
        for neighbor in graph[node]:
            if neighbor in pq:
                new_score = dist[node] + graph[node][neighbor]
                if new_score < pq[neighbor]:
                    pq[neighbor] = new_score
                    pred[neighbor] = node
    return dist, pred


if __name__ == "__main__":
    rail = LondonRailStations (tube=False)
    print "There are %s rail stations" % len (rail.latlons)
    rail.makeDMat ()
    np.savetxt ("../data/London-rail-station-dists.txt", rail.dmat,
            delimiter=',')
    with open('../data/London-rail-station-names.txt', 'wb') as outfile:
        wr = csv.writer (outfile, delimiter='\n', quoting=csv.QUOTE_ALL)
        wr.writerow (rail.dmatNames)
    print 'Distance matrix of %s stations written to' % rail.dmat.shape[0],
    print 'London-rail-station-dists.txt'
    print '\talong with vector of London-rail-station-names.txt' 

    tube = LondonRailStations (tube=True)
    print "There are %s tube stations" % len (tube.latlons)
    tube.makeDMat ()
    np.savetxt ("../data/London-tube-station-dists.txt", tube.dmat,
            delimiter=',')
    with open('../data/London-tube-station-names.txt', 'wb') as outfile:
        wr = csv.writer (outfile, delimiter='\n', quoting=csv.QUOTE_ALL)
        wr.writerow (tube.dmatNames)
    print 'Distance matrix of %s stations written to' % tube.dmat.shape[0],
    print 'London-tube-station-dists.txt'
    print '\talong with vector of London-tube-station-names.txt' 

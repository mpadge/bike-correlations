#!/usr/bin/python

# Utility scripts to zip and unzip the raw data files. Zip is used because the
# NYC data come zipped.

import os, subprocess, sys, getopt

def zipFiles (city="London"):
    wd0 = '/data/data/bikes/'
    if city == 'London':
        wd = wd0 + 'barclayscyclehireusagestats/'
    else:
        wd = wd0 + 'nyc/'
    dir = os.listdir (wd)
    for d in dir:
        # Only compress .csv files:
        if d.find (".csv") > -1:
            fout = wd + d.split(".csv")[0] + '.zip'
            # but don't do it if .zip already exists
            if not os.path.exists (fout):
                fin = wd + d
                args = ["zip", fout, fin]
                subprocess.Popen (args)

def unzipFiles (city="London"):
    wd0 = '/data/data/bikes/'
    if city == 'London':
        wd = wd0 + 'barclayscyclehireusagestats/'
    else:
        wd = wd0 + 'nyc/'
    dir = os.listdir (wd)
    for d in dir:
        if d.find (".zip") > -1:
            fz = wd + d
            args = ["unzip", fz, "-d", wd]
            subprocess.Popen (args)

def removeFiles (city="london", files="csv"):
    wd0 = '/data/data/bikes/'
    if city == 'London':
        wd = wd0 + 'barclayscyclehireusagestats/'
    else:
        wd = wd0 + 'nyc/'
    dir = os.listdir (wd)
    if files == "csv":
        sfx = ".csv"
    else:
        sfx = ".zip"
    for d in dir:
        frm = wd + d
        if d.find (sfx) > -1:
            print "removed file ", frm
            args = ["rm", frm]
            subprocess.Popen (args)

if __name__ == "__main__":
    opts, args = getopt.getopt (sys.argv[1:],[])
    if len (args) < 2:
        print "usage: Utils <remove, zip, unzip> <london/nyc> <files to remove>"
    else:
        if args [1].lower ().find ("lond") > -1:
            city = "london"
        else:
            city = "nyc"
        if args[0] == 'remove':
            if len (args) > 2:
                files = args [2]
            else:
                files = "csv"
            tstr = 'Removing all ' + files + ' files from ' + city + '. Okay? '
            ans = raw_input (tstr)
            if ans.lower ()[0] == 'y':
                removeFiles (city, files)
        elif args[0] == 'zip':
            tstr = 'zipping all files from ' + city + '. Okay? '
            ans = raw_input (tstr)
            if ans.lower ()[0] == 'y':
                zipFiles (city)
        elif args[0] == 'unzip':
            tstr = 'UNzipping all files from ' + city + '. Okay? '
            ans = raw_input (tstr)
            if ans.lower ()[0] == 'y':
                unzipFiles (city)

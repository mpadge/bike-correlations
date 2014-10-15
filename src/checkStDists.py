import pandas as pd
import numpy as np
import shutil as sh
import os, argparse

def removeDuplicates (fname):
    # Fist make copy of original
    fname_old = fname.split ('.txt')[0] + '_old.txt'
    sh.copyfile (fname, fname_old)

    dat = pd.read_csv(fname, names=['from','to','d'])
    grouped = dat.groupby (['from','to'])
    index = [gp_keys[0] for gp_keys in grouped.groups.values()]
    dat2 = dat.reindex(index).sort(['from','to'])
    ndups = len (dat) - len (dat2)
    print "%s / %s duplicate entries removed from %s"\
            % ('{0:,}'.format(ndups), '{0:,}'.format(len(dat)),\
            os.path.basename(fname))
    # Original has only 3-digit floats, but pandas converts to full floats. The
    # next line saves memory by rounding back
    dat2 ['d'] = np.round (dat['d'].convert_objects(convert_numeric=True), 
            decimals=3)
    print "written to %s" % fname
    dat2.to_csv(fname,index=False, header=False)

def rootDir ():
    return os.getcwd ().split ('src/')[0]
    
    
def findMissingDists (city):
    # There are no missing trips, so no action need be taken at present.
    fname = rootDir () + '/data/station_latlons_' + city + '.txt'
    csv = pd.read_csv (fname)
    st = pd.DataFrame ({
        'id': csv ['id'],
        'lat': csv [' lat'],
        'lon': csv [' long'],
        'name': csv [' name']})
    n = len (st)
    stcheck = np.zeros ([n, n])
    # Then make a reverse lookup table to replace station numbers with corresponding
    # [row,col] numbers in stcheck
    di = st.loc[:,['id']].to_dict()['id']
    direv = dict ([(v,k) for k,v in di.iteritems()])
    dname = rootDir () + '/results/station_dists_' + city + '.txt'
    dat = pd.read_csv (dname, names=['from','to','d'])
    stfrom = dat ['from'].replace (direv)
    stto = dat ['to'].replace (direv)
    stcheck [np.array([stfrom, stto])] = 1
    numdone = np.count_nonzero (stcheck)
    # numdone should equal n * (n - 1)
    if numdone != (n * (n - 1)):
        print "%s / %s inter-station comparisons have distances; %s are missing"\
                % (numdone, stcheck.size, stcheck.size - numdone)
    else:
        print "all inter-station distances exist"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Removes duplicate stdists')
    parser.add_argument('city', nargs='?',\
        help='Specify the city to perform analysis: nyc or (default) london')
    args = parser.parse_args()
    if not args.city: args.city = "london"

    fname = 'results/station_dists_' + args.city + '.txt'
    removeDuplicates (fname)
    print "finding missing trips (may take a short while) ..."
    findMissingDists (args.city)

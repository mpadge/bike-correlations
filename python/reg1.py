"""
Author: Colin Broderick
"""
import pandas as pd
import numpy as np
import scipy as sp
import scipy.stats
import argparse

# Open the trips from file
print "Loading Trips."
boom1 = pd.read_csv('total_from.csv', header=1, skiprows=[2])
boom1 = boom1.set_index('end_id')
# want to get linear regression between first and second row.
"""xi = boom1.irow(1)
y = boom1.irow(2)
mask = ~np.isnan(xi) & ~np.isnan(y)
slope, intercept, r_value, p_value, std_err = sp.stats.linregress(xi[mask],y[mask])

#access the columns
boom1.iloc[:,]

#access the row
boom1.iloc[1]"""

print "Calculating correlations"
row_r2vals = []
# get r2 values of each row with the next row
for row in range(0, len(boom1)-1):
    xi = boom1.iloc[row]
    y = boom1.iloc[row+1]
    mask = ~np.isnan(xi) & ~np.isnan(y)
    slope, intercept, r_value, p_value, std_err = sp.stats.linregress(xi[mask],y[mask])
    row_r2vals.append(r_value**2)

col_r2vals = []
# get r2 values for columns with each column and the next column.
for col in range(0, len(boom1.columns)-1):
    print col
    xic = boom1.iloc[:,col]
    yc = boom1.iloc[:,col+1]
    mask = ~np.isnan(xic) & ~np.isnan(yc)
    slope, intercept, r_value, p_value, std_err = sp.stats.linregress(xic[mask],yc[mask])
    col_r2vals.append(r_value**2)

print "No of R2values: %s x %s" % (len(row_r2vals), len(row_r2vals))
import numpy as np
import math
import matplotlib.pyplot as plt
import fileinput
from matplotlib.collections import PolyCollection
from matplotlib import cm
import sys
import re

name = sys.argv[1]
f = 
first_line = fileinput.input().readline()
if data_type == 'm':
	values = list()
	for i in first_line[:-1]: res.append(int(i))
else:
	values = [int(x) for x in first_line.rsplit(None, 1)[0].split(',')]

root = fileinput.filename()[:-1]
params = open(root+'p', 'r')
for line in params:
	exec line

def HexCollection(m, n, fc):
	r = 1/math.sqrt(3) # distance from center point to vertex
	h = 3*r/2
	verts = list()
	for pos in xrange(m*n):
		row = pos / n; col = pos % n # position in mesh
		cx = 0.5 + col + (row%2)*0.5; cy = r + row*h # location of center point
		verts.append([(cx-0.5, cy-r/2),(cx-0.5, cy+r/2),(cx, cy+r),(cx+0.5, cy+r/2),(cx+0.5, cy-r/2),(cx, cy-r)])	
	return PolyCollection(verts, edgecolors = ('0.75',), facecolors = fc, linewidths = (0.5,))

maxV = float(max(values))
fc = [cm.jet(V/maxV) for V in values]
hexV = HexCollection(m, n, fc)
fig = plt.figure()
ax = fig.gca()
ax.add_collection(hexV)
ax.set_xlim(0,n+0.5)
ax.set_ylim(0, (m-1)*math.sqrt(3)/2 + 2/math.sqrt(3))
ax.set_aspect('equal')
ax.axis('off')
#plt.show()
plt.savefig('grph/'+fileinput.filename()[4:]+'.pdf')

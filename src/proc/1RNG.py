from __future__ import print_function
import lattice, libRNG, sys

#rid='overdispersed_large'
#resdir='/home/colin/projects/SEED/int'

resdir = str(sys.argv[1])
rid=str(sys.argv[2])
dump=bool(sys.argv[3])

Hex=lattice.lattice(50,50,wrap=False)
Hex.fillNYC()

canopy = lattice.getEndCanopy(resdir, rid)

DT = libRNG.getDelaunayGraph(Hex, canopy)
RNG = libRNG.getRelativeNeighborGraph(Hex, DT)

#import matplotlib.pyplot as plt
#import matplotlib.collections as coll
#from matplotlib import cm
#Hex.plot(plt, coll, [1 if i in canopy else 0 for i in xrange(Hex.N)], [cm.binary(1.0) if i in canopy else cm.binary(0.0) for i in xrange(Hex.N)],[cm.binary(0) for x in xrange(Hex.N)], (0,), '', net=RNG)

D,avgMIJ = libRNG.getRNGstats(Hex, canopy, RNG)
maxMij= libRNG.getExtent(Hex, canopy)

if dump:
	f=open(resdir+'/RNG.'+rid, 'w')
	for i,j,w in RNG.edges(data=True):
		print(i,j,w['weight'], file=f)
	f.close()

print(maxMij, D, avgMIJ)

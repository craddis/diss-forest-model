from scipy.spatial import Delaunay, ConvexHull
import networkx as nx
from itertools import combinations
from numpy import mean, var

def getCompleteGraph(H, canopy): # H=lattice object , canopy=focal snake list
	output = nx.Graph() # Create empty graph
	for u,v in combinations(canopy,2): # For all node pairs...
		output.add_edge(u,v,weight=H.getNYC(u,v)) # Add distance-weighted edge
	return output

def getDelaunayGraph(H, canopy): # H=lattice object , canopy=focal snake list
	canopy = list(canopy)
	V = [H.centers[snake] for snake in canopy] # get central coordinates for each focal tree
	output = nx.Graph() # Create empty graph
	for triangle in Delaunay(V).simplices: # Add all Delaunay triangles to graph
		output.add_edge(canopy[triangle[0]],canopy[triangle[1]], weight = H.getNYC(canopy[triangle[0]],canopy[triangle[1]]))
		output.add_edge(canopy[triangle[1]],canopy[triangle[2]], weight = H.getNYC(canopy[triangle[1]],canopy[triangle[2]]))
		output.add_edge(canopy[triangle[0]],canopy[triangle[2]], weight = H.getNYC(canopy[triangle[0]],canopy[triangle[2]]))
	return output

def getRelativeNeighborGraph(H, DT): # H:lattice object , DT:Delaunay nx.Graph object
	output = nx.Graph() # Create empty graph
	output.add_weighted_edges_from(((i,j,w['weight']) for i,j,w in DT.edges(data=True))) # Add all edges from Delaunay Graph
	for v in output.nodes():
		for i,j in output.edges():
			if H.getNYC(i,j) > max(H.getNYC(v,i),H.getNYC(v,j)):
				output.remove_edge(i,j)
	return output

def getExtent(H, canopy):
	return max((H.getNYC(i,j) for i,j in combinations(canopy,2)))

def getRNGstats(H, canopy, RNG):
	asplRNG = nx.all_pairs_dijkstra_path_length(RNG, weight="weight")
	diamRNG = max((asplRNG[s][t] for s,t in combinations(canopy,2)))
	periRNG = [(s,t) for s,t in combinations(canopy,2) if asplRNG[s][t]==diamRNG]
	MIJ = mean([H.getNYC(s,t) for s,t in periRNG])
	return diamRNG, MIJ


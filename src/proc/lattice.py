# Version 3.4
from math import sqrt, degrees, atan
from copy import copy

class lattice(): # Complete Hexagonal Lattice covering a rectangular area
	def __init__(self, ncol, nrow, wrap=False, size=1/sqrt(3), convention='odd-r', rotation='pointy-top'):
		self.ncol = ncol
		self.nrow = nrow
		self.N = ncol * nrow
		self.size = size
		self.offset = (sqrt(3)/2*size, size)
		self.convention = convention
		self.rotation = rotation
		self.height = size * 2
		self.width = sqrt(3) / 2 * self.height
		self.vertical = 0.75 * self.height
		self.horiz = self.width
		self.wrap = wrap
		self.NYC = {}
		self.all_vertices = [self.vertices(i) for i in xrange(self.N)]
		self.centers = dict()
		# fill centers
		for i in xrange(self.N): self.centers[i] = self.center(i)
	def outside(self, qr): # TESTED
		return qr[0]<0 or qr[0]>=self.ncol or qr[1]<0 or qr[1]>=self.nrow
	def snake2qr(self, snake, dir=0): # TESTED
		q = snake % self.ncol
		r = snake / self.ncol
		if dir==0:
			return q, r
		elif dir==1:
			return q, r+self.nrow
		elif dir==2:
			return q+self.ncol, r+self.nrow
		elif dir==3:
			return q+self.ncol, r
		elif dir==4:
			return q+self.ncol, r-self.nrow
		elif dir==5:
			return q, r-self.nrow
		elif dir==6:
			return q-self.ncol, r-self.nrow
		elif dir==7:
			return q-self.ncol, r
		elif dir==8:
			return q-self.ncol, r+self.nrow
		else:
			return q, r
	def qr2cube(self, qr):
		q = qr[0]; r = qr[1]
		odd = abs(r % 2)
		x = q - (r - odd) / 2
		z = r
		y = -x - z
		return x, y, z
	def cube2qr(self, cube): # TESTED
		odd = abs(cube[2] % 2)
		q = cube[0] + (cube[2] - odd) / 2
		r = cube[2]
		return q, r
	def qr2snake(self, qr): # TESTED
		q = qr[0]; r = qr[1]
		if self.outside(qr):
			if self.wrap:
				if q < 0:
					q = self.ncol + q
				if q >= self.ncol:
					q = q - self.ncol
				if r < 0:
					r = self.nrow + r
				if r >= self.nrow:
					 r = qr[1] - self.nrow
				return r * self.ncol + q
			else:
				return -1
		else:
			return r * self.ncol + q
	def cubeRound(self, cube):
		rx = int(round(cube[0]))
		ry = int(round(cube[1]))
		rz = int(round(cube[2]))
		x_diff = abs(rx-cube[0])
		y_diff = abs(ry-cube[1])
		z_diff = abs(rz-cube[2])
		if x_diff > y_diff and x_diff > z_diff:
			rx = -ry-rz
		elif y_diff > z_diff:
			ry = -rx-rz
		else:
			rz = -rx-ry
		return rx, ry, rz
	def line(self, orig, dest, dir=0, cache=False):
		output = []
		cubeOrig = self.qr2cube(self.snake2qr(orig))
		cubeDest = self.qr2cube(self.snake2qr(dest, dir))
		D = self.cubicDist(cubeOrig, cubeDest)
		for i in xrange(D+1):
			prop = i/float(D)
			x = cubeOrig[0] + (cubeDest[0]-cubeOrig[0]) * prop
			y = cubeOrig[1] + (cubeDest[1]-cubeOrig[1]) * prop
			z = cubeOrig[2] + (cubeDest[2]-cubeOrig[2]) * prop
			output.append(self.qr2snake(self.cube2qr(self.cubeRound([x,y,z]))))
		return output
	def cubicDist(self, orig, dest):
		dx = abs(orig[0] - dest[0])
		dy = abs(orig[1] - dest[1])
		dz = abs(orig[2] - dest[2])
		return max([dx, dy, dz])
	def manhattan(self, orig, dest):
		origCube = self.qr2cube(self.snake2qr(orig))
		if self.wrap:
			distances = list()
			for i in xrange(9):
				distances.append(
					self.cubicDist(origCube, self.qr2cube(self.snake2qr(dest, i)))
				)
			return min(distances)
		else:
			return self.cubicDist(origCube, self.qr2cube(self.snake2qr(dest)))
	def project(self, orig, dir, dist): # TESTED
		x = orig[0]; y = orig[1]; z = orig[2];
		if dir==0: z+=dist; y-=dist;
		elif dir==1: x+=dist; y-=dist;
		elif dir==2: x+=dist; z-=dist;
		elif dir==3: y+=dist; z-=dist;
		elif dir==4: x-=dist; y+=dist;
		elif dir==5: x-=dist; z+=dist;
		#else: BREAK
		return x,y,z
	def fillNYC(self):
		for i in xrange(self.N):
			self.NYC[i] = {}
			for j in xrange(i, self.N):
				self.NYC[i][j] = self.manhattan(i, j)
	def getNYC(self, orig, dest):
		if orig <= dest:
			return self.NYC[orig][dest]
		else:
			return self.NYC[dest][orig]
	def ring(self, pos_snake, R): # TESTED
	#What if wrapped ring double counts a cell?
		output = []
		pos_cube = self.project(self.qr2cube(self.snake2qr(pos_snake)), 4, R)
		for i in xrange(6):
			for j in xrange(R):
				pos_cube = self.project(pos_cube, i, 1)
				pos_snake = self.qr2snake(self.cube2qr(pos_cube))
				if pos_snake != -1:
					output.append(pos_snake)
		return output
	def center(self, snake):
		q,r = self.snake2qr(snake)
		odd = r % 2
		x = self.offset[0] + q*self.horiz + odd*self.width/2
		y = self.offset[1] + r*self.vertical
		return x, y
	def fillCenters(self):
		for i in xrange(self.N):
			self.centers[i] = self.center(i)
	def vertices(self, snake):
		x,y = self.center(snake)
		twelve = [ x               , y + self.height/2 ]
		two    = [ x + self.width/2, y + self.height/4 ]
		four   = [ x + self.width/2, y - self.height/4 ]
		six    = [ x               , y - self.height/2 ]
		eight  = [ x - self.width/2, y - self.height/4 ]
		ten    = [ x - self.width/2, y + self.height/4 ]
		return twelve, two, four, six, eight, ten
	def segmentize(self, snakes):
		output = list()
		for i in xrange(len(snakes)-1):
			output.append((self.center(snakes[i]), self.center(snakes[i+1])))
		return output
	def xydist(self, orig, dest): # Snake -> Euclidean distance (no wrap)
		dx = self.centers[orig][0]-self.centers[dest][0]
		dy = self.centers[orig][1]-self.centers[dest][1]
		return sqrt(dx*dx + dy*dy)
	def angle(self, orig, dest):
		dx = self.centers[dest][0]-self.centers[orig][0]
		dy = self.centers[dest][1]-self.centers[orig][1]
		if dx > 0:
			return degrees(atan(dy/dx)) + (dy < 0)*360
		elif dx < 0:
			return degrees(atan(dy/dx)) + 180
		else:
			return 90.0 + (dy<0)*180
	def plot(self, plt, coll, val, fc, ec, lw, title, net=None):
		fig = plt.figure()
		ax = fig.gca()
		poly = coll.PolyCollection(self.all_vertices, edgecolors=ec, facecolors=fc, linewidths=lw)
		ax.set_title(title+'\n'+str(round(min(val), 2))+' -> '+str(round(max(val), 2)))
		ax.add_collection(poly)
		ax.set_xlim(0, self.offset[0] + self.ncol*self.width)
		ax.set_ylim(0, self.offset[1] + (self.nrow-1)*self.vertical + self.height/2)
		ax.set_aspect('equal')
		ax.axis('off')
		if net:
			segments=[]
			for edge in net.edges_iter():
			    segments.append((self.center(edge[0]), self.center(edge[1])))
			Lcoll = coll.LineCollection(segments, linewidths=(1,))
			ax.add_collection(Lcoll)

class plant:
	def __init__(self, id, species, parent, disp):
		self.id = id
		self.parent = parent
		self.species = species
		self.fruit = False
		self.size = -1
		self.est = -1
		self.disp = disp

class cell:
	def __init__(self, snake):
		self.snake = snake
		self.tree = plant(-1, False, -1, False)
		self.bank = []

class agent:
	def __init__(self):
		self.snake = -1
		self.trees = 0
		self.longM = []
		self.shortM = []

def loadTracks(path, rid, A):
	output = []
	for i in xrange(A): output.append([])
	file = open(path+'/tracks.'+str(rid))
	for line in file:
		data = [int(i) for i in line.split(' ')]
		output[data[1]].append(data[2:])
	return output

def getTreeIDs(path, rid, year):
	ids = list()
	file = open(path+'/trees'+str(rid))
	targetDay = year*365
	for line in file:
		val = [int(x) for x in line[:-1].split(' ')]
		if val[4] >= targetDay:
			ids[val[0]] = val[1]
	return ids	

def getChildParentMap(path, rid, species=1):
	file = open(path+'/trees.'+str(rid))
	output = {}
	for line in file:
		temp = [int(x) for x in line[:-1].split(' ')]
		if bool(temp[2])==species:
			output[temp[1]] = temp[3]
	return output

def getEndCanopy(path, rid, snake=True):
	output = set()
	file = open(path+'/fruit.'+str(rid))
	for line in file:
		temp = [int(x) for x in line[:-1].split(' ')]
		if temp[2]==1:
			if snake:
				output.add(temp[0])
			else:
				output.add(temp[1])
	return output

def getAllCanopy(path, rid):
	output = list()
	file = open(path+'/trees.'+str(rid))
	current_year = 0;
	canopy = set()
	for line in file:
		temp = [int(x) for x in line[:-1].split(' ')]
		emrg_year = temp[4] / 365 + (temp[4]>=0)
		if emrg_year > current_year:
			output.append(copy(canopy))
			current_year = emrg_year
		if temp[2]==1:
			canopy.add(temp[0])
		elif temp[0] in canopy:
			canopy.remove(temp[0])			
	output.append(copy(canopy))
	return output


def loadPop(rid):
	population = []
	for i in xrange(20): population.append(agent())
	file = open('/export/scratch/colin/pop.state.'+str(rid))
	aid = 0
	for line in file:
		x = line.split(' ')
		population[aid].snake = int(x[0])
		population[aid].trees = float(x[1])
		population[aid].longM = [float(y) for y in x[2:2502]]
		population[aid].shortM = [int(y) for y in x[2502:-1]]
		aid = aid + 1
	return population


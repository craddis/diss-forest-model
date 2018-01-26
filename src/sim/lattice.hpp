// Version 3.5
#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <cmath>
#include <algorithm>
#include "random.hpp"
#define PI 3.14159265

using namespace std;
using namespace keittlab;

struct lattice { // Complete Hexagonal Lattice covering a rectangular area
	int ncol;
	int nrow;
	int N;
	double size;
	array<double,2> offset;
	string convention;
	string rotation;
	double height;
	double width;
	double vertical;
	double horiz;
	unordered_map< int, vector<int> > adjlist;
	unordered_map< int, unordered_map<int,int> > NYC;
	bool wrap;
	lattice(int NCOL, int NROW, bool WRAP=false, double SIZE=1/sqrt(3), string CONVENTION="odd-r", string ROTATION="pointy-top") {
		ncol = NCOL;
		nrow = NROW;
		N = NCOL * NROW;
		size = SIZE;
		offset[0] = sqrt(3)/2*SIZE;
		offset[1] = SIZE;
		convention = CONVENTION;
		rotation = ROTATION;
		height = SIZE * 2;
		width = sqrt(3) / 2 * height;
		vertical = 0.75 * height;
		horiz = width;
		wrap = WRAP;
	}
	bool outside(array<int,2> qr) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		return qr[0]<0 || qr[0]>=ncol || qr[1]<0 || qr[1]>=nrow;
	}
	array<int,2> qr(int snake, int dir=0) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		int q = snake % ncol;
		int r = snake / ncol;
		array<int,2> out;
		if(dir==0) {out[0]=q; out[1]=r;}
		else if(dir==1) {out[0]=q;	out[1]=r+nrow;}
		else if(dir==2) {out[0]=q+ncol; out[1]=r+nrow;}
		else if(dir==3) {out[0]=q+ncol; out[1]=r;}
		else if(dir==4) {out[0]=q+ncol; out[1]=r-nrow;}
		else if(dir==5) {out[0]=q; out[1]=r-nrow;}
		else if(dir==6) {out[0]=q-ncol; out[1]=r-nrow;}
		else if(dir==7) {out[0]=q-ncol; out[1]=r;}
		else if(dir==8) {out[0]=q-ncol; out[1]=r+nrow;}
		else {out[0]=q; out[1]=r;}
		return out;
	}
	array<int,3> cube(array<int,2> qr) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		int q = qr[0]; int r = qr[1];
		bool odd = abs(r % 2);
		int x = q - (r - odd) / 2;
		int z = r;
		int y = -x - z;
		array<int,3> output {{ x , y , z }};
		return output;
	}
	array<int,2> qr(array<int,3> cube) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		int odd = abs(cube[2] % 2);
		int q = cube[0] + (cube[2] - odd) / 2;
		int r = cube[2];
		array<int,2> output {{ q, r }};
		return output;
	}
	int snake(array<int,2> qr) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		if(outside(qr)) {
			if(wrap) {
				if(qr[0]<0) qr[0] = ncol+qr[0];
				if(qr[0]>=ncol) qr[0] = qr[0]-ncol;
				if(qr[1]<0) qr[1] = nrow+qr[1];
				if(qr[1]>=nrow) qr[1] = qr[1]-nrow;
				return qr[1] * ncol + qr[0];
			}
			else return -1;
		}
		else return qr[1] * ncol + qr[0];
	}
	int cubicDist(array<int,3> orig, array<int,3> dest) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		int dx = abs(orig[0] - dest[0]);
		int dy = abs(orig[1] - dest[1]);
		int dz = abs(orig[2] - dest[2]);
		return (dx+dy+dz) / 2;
	}
	int manhattan(int orig, int dest) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		auto origCube = cube(qr(orig));
		if(wrap) {
			vector<int> distances;
			for(int i=0; i!=9; ++i) {
				distances.push_back(cubicDist(origCube, cube(qr(dest, i))));
			}
			return *min_element(distances.begin(), distances.end());
		}
		else return cubicDist(origCube, cube(qr(dest)));
	}
	array<int,3> project(array<int,3> orig, int dir, int dist) { // VERIFIED
		int x = orig[0]; int y = orig[1]; int z = orig[2];
		if(dir == 0) {z+=dist; y-=dist;}
		else if(dir == 1) {x+=dist;y-=dist;}
		else if(dir == 2) {x+=dist;z-=dist;}
		else if(dir == 3) {y+=dist;z-=dist;}
		else if(dir == 4) {x-=dist;y+=dist;}
		else if(dir == 5) {x-=dist;z+=dist;}
		array<int,3> output {{ x, y, z }};
		return output; // What if dir > 5?
	}
	void fillNYC() {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		for(int i=0; i!=N; ++i) {
			for(int j=i; j!=N; ++j) {
				NYC[i][j] = manhattan(i, j);
			}
		}
	}
	int getNYC(int orig, int dest) {
		// SUCCESSFUL NO WRAP TEST 2015-12-01
		if(orig<=dest) return NYC[orig][dest];
		else return NYC[dest][orig];
	}
	vector<int> ring(int pos_snake, int R) { // VERIFIED
		vector<int> output;
		auto pos_cube = project(cube(qr(pos_snake)), 4, R);
		for(int i=0; i!=6; i++) {
			for(int j=0; j!=R; ++j) {
				pos_cube = project(pos_cube, i, 1);
				pos_snake = snake(qr(pos_cube));
				if(pos_snake != -1) output.push_back(pos_snake);
			}
		}
		return output;
	}
	void fillAdj() { // VERIFIED
		for(int i=0; i!= N; ++i) adjlist.emplace(i, ring(i, 1));
	}
	int direction(int origin, int dest) { //
		auto dist = getNYC(origin, dest);
		auto adj = adjlist[origin];
		vector<int> choices;
		for(auto& i : adj) {
			if(i >= 0 && getNYC(i, dest) < dist) {
				choices.push_back(i);
			}
		}
		//return *one_of(choices);
		return choices[0];
	}
	int rwalk(int snake, int nsteps) { // UNTESTED BUT ASSUME IT WORKS
		for(int i=0; i!= nsteps; ++i) snake = *one_of( adjlist.find(snake)->second );
		return snake;
	}
	/**
	array<double, 2> xy(int snake) { // UNTESTED
		int q = snake % ncol;
		int r = snake / ncol;
		int odd = r % 2;
		double x = offset[0] + q*horiz + odd*width/2.0;
		double y = offset[1] + r*vertical;
		array<double,2> output {{ x,y }};
		return output;
	}
	double angle(int origin, int dest) {  // UNTESTED
		auto oxy = xy(origin);
		auto dxy = xy(dest);
		double dx = dxy[0]-oxy[0];
		double dy = dxy[1]-oxy[1];
		double theta = atan(dy/dx) * 180 / PI;
		if(dx < 0) theta += 180;
		else if(dy < 0) theta += 360;
		return theta;
	}
	Mat<int> getNYC() {
		Mat<int> output = zeros<Mat<int>>(N, N);
		for(int i = 0; i != N; ++i) {
			for(int j = 0; j != N; ++j) {
				output(i, j) = manhattan(i, j);
				output(j, i) = output(i, j);
			}
		}
		return output;
	}
	**/
	/**vector<int> ringA(int snake, int R) { // VERIFIED
		vector<int> output;
		for(int i=0; i!=N; ++i) {
			if(manhattan(snake, i) == R) output.push_back(i);
		}
		return output;
	}
	int direction(int origin, int dest) {
		auto O = cube(origin);
		auto D = cube(dest);
		int dx = O[0] - D[0];
		int dy = O[1] - D[1];
		int dz = O[2] - D[2];
		int a = abs( dx - dy );
		int b = abs( dy - dz );
		int c = abs( dz - dx );
		auto adj = getAdj(origin, true, false);
		if(a >= b) {
			if(a > c) {
				if(x > 0) return 12;
				else return 9;
			}
			else {
				if(z > 0) return 7;
				else return 1;
			}
		}
		else {
			if(b >= c) {
				if(y > 0) return 11;
				else return 5;
			}
			else {
				if(z > 0) return 7;
				else return 1;
			}
		}
	}**/
};

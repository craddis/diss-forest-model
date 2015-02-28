//#include <vector> 
//#include <string>
//#include <unordered_map
//#include <array>
//#include <cmath>
//#include "random.hpp"

//using namespace std;
//using namespace keittlab;

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
	lattice(int NCOL, int NROW, double SIZE=1/sqrt(3), string CONVENTION="odd-r", string ROTATION="pointy-top") 		{
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
	}
	int snake(array<int,2> qr) { // VERIFIED
		return qr[1] * ncol + qr[0];
	}
	array<int,2> qr(int snake) { // VERIFIED
		int q = snake % ncol;
		int r = snake / ncol;
		array<int,2> output {{ q , r}};
		return output;
	}
	array<int,2> qr(array<int,3> cube) { // VERIFIED
		int odd = cube[2] % 2; 
		int q = cube[0] + (cube[2] - odd) / 2;
		int r = cube[2];
		array<int,2> output {{ q, r }};
		return output;
	}
	array<int,3> cube(int snake) { // VERIFIED
		int q = snake % ncol;
		int r = snake / ncol;
		bool odd = r % 2;
		int x = q - (r - odd) / 2;
		int z = r;
		int y = -x - z;
		array<int,3> output {{ x , y , z }};
		return output;
	}
	int manhattan(int snakeA, int snakeB) { // VERIFIED
		auto A = cube(snakeA);
		auto B = cube(snakeB);
		int dx = abs(A[0] - B[0]);
		int dy = abs(A[1] - B[1]);
		int dz = abs(A[2] - B[2]);
		return (dx+dy+dz) / 2;
	}
	array<int,3> project_cube(array<int,3> start, int dir, int dist) { // VERIFIED
		int x = start[0]; int y = start[1]; int z = start[2];
		if(dir == 0) {z+=dist; y-=dist;}
		else if(dir == 1) {x+=dist;y-=dist;}
		else if(dir == 2) {x+=dist;z-=dist;}
		else if(dir == 3) {y+=dist;z-=dist;}
		else if(dir == 4) {x-=dist;y+=dist;}
		else if(dir == 5) {x-=dist;z+=dist;}
		array<int,3> output {{ x, y, z }};
		return output;
	}
	vector<int> ring(int center, int R) { // VERIFIED
		vector<int> output;
		auto CUBE = cube(center);
		auto QR = qr(center);
		CUBE = project_cube(CUBE, 4, R);
		for(int i=0; i!=6; i++) {
			for(int j=0; j!=R; ++j) {
				CUBE = project_cube(CUBE, i, 1);
				QR = qr(CUBE);
				if(QR[0]>=0 && QR[0]<ncol && QR[1]>=0 && QR[1]<nrow) {
					output.push_back(snake(QR));
				}
			}
		}
		return output;
	}
	int direction(int origin, int dest) { // VERIFIED
		auto dist = manhattan(origin, dest);
		auto adj = getAdj(origin);
		vector<int> choices;
		for(auto& i : adj) {
			if(i >= 0 && manhattan(i, dest) < dist) {
				choices.push_back(i);
			}
		}
		return *one_of(choices);
	}
	int rwalk(int snake, int nsteps) {
		for(int i=0; i!= nsteps; ++i) snake = *one_of( adjlist.find(snake)->second );
		return snake;
	}
	/**array<double,2> cartesian(int snake) {
		int q = qr(snake)[0];
		int r = qr(snake)[1];
		bool odd = r % 2;
		double x = offset[0] + odd*width/2 + q*horiz;
		double y = offset[1] + q*vertical;
		array<double,2> output {{ x , y }};
		return output;
	}**/
	/**Mat<int> getNYC() {
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
	vector<int> getAdj(int snake, bool fill_empty=false, bool include_self=false) { 
		// convention: odd rows shift right
		int q = qr(snake)[0];
		int r = qr(snake)[1];
		bool odd = r % 2;
		std::vector<int> adj;
		if(r < nrow-1 && (!odd || q < ncol-1)) adj.push_back(snake + ncol + odd);    // 1 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(q < ncol-1                        ) adj.push_back(snake + 1);             // 3 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(r > 0      && (!odd || q < ncol-1)) adj.push_back(snake - ncol + odd);    // 5 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(r > 0      && ( odd || q > 0)     ) adj.push_back(snake - ncol - 1 + odd);// 7 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(q > 0                             ) adj.push_back(snake - 1);             // 9 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(r < nrow-1 && ( odd || q > 0)     ) adj.push_back(snake + ncol - 1 + odd);// 11 o'clock
		else if(fill_empty) adj.push_back(-1);
		if(include_self) adj.push_back(snake);
		return adj;
	}
	void fillAdj() {
		for(int i=0; i!= N; ++i) adjlist.emplace(i, getAdj(i));
	}
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

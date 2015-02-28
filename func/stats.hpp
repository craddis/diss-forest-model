// #include <cmath>
// #include "lattice.hpp"
// #include "defs.hpp"

double join_count(const landscape& L, lattice& H) {
// See "Spatial Methods for Data Analysis", Shabenberger and Gotway, pg 19-21
	double BB = 0; double w = 0; double S2 = 0; double nB = 0;
	for(auto& C : L) {
		if(C.tree.species) nB++;
		auto adj = H.getAdj(C.snake);
		S2 += pow(2.0*adj.size(), 2.0);
		for(auto& i : adj) {
			BB += 0.5*C.tree.species * L[i].tree.species;
			w++;
		}
	}
	double S1 = 2.0*w;
	double P2 = nB*(nB-1)/(H.N*(H.N-1));
	double P3 = P2*(nB-2)/(H.N-2);
	double P4 = P3*(nB-3)/(H.N-3);
	double EBB = 0.5*P2*w;
	double VarBB = 0.25*(S1*(P2-2*P3+P4)+S2*(P3-P4)+w*w*P4-pow(w*P2, 2.0));
	
	double z_score = (BB-EBB)/VarBB;
	return z_score;
}

double abundance(lattice& H, state_var& SV) {
	return SV.Nf*1.0/H.N;
}

// Scofield 2012
/**
double rgg(const vector<plant>& bank, bool species) {
	int n = 0;
	vector<int> unique_mothers;
	for(auto& P : B) {
		
	}
}**/


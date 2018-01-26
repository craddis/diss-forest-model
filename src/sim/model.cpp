// Version 3.5
#include "io.hpp" // Brings in defs.hpp, lattice.hpp, random.hpp

using namespace std;
using namespace keittlab;

int main(int argc, char* argv[]) {

//MODEL INITIALIZATION
randomize();
parameters par(argv[1]);
lattice hex(par.R, par.Q, par.wrap);
hex.fillAdj();
hex.fillNYC();
binomial_distribution<> ngaps(hex.N, par.amort);
uniform_real_distribution<double> urd(0,1);
state counts;
landscape forest(hex.N); // Vector of empty cells/trees objects
population foragers(par.A);
vector<int> indeces;
for(int i=0; i!=hex.N; ++i) indeces.push_back(i);

ofstream treesfile(par.resdir+"/trees."+par.rid);

InitLandRand(hex, forest, par, counts, urd, treesfile); // Place trees randomly
InitPopRand(hex, foragers); // Place agents randomly

if(par.naive==false) {
	inform(forest, foragers[0], par);
	for(auto it=foragers.begin()+1; it!=foragers.end(); ++it) it->memory = foragers[0].memory;
}

// SIMULATION
for(; counts.day != par.days; ++counts.day) {
	int startFruit = totalFruit(forest, true);
	shuffle(foragers.begin(), foragers.end(), urng());
	for(int step = 0; step != par.steps; ++step) {
		// TIME STEP
		for(auto& A : foragers) takeStep(A, forest, hex, par, counts);
	}
	// END OF THE DAY
	int endFruit = totalFruit(forest, true);
	for(auto& A : foragers) predict(A, forest, par);
	if(par.sleep) {
		for(auto& A : foragers) seedflush(A, forest, counts); // Void all seeds at sleeping site
	}
	matrixDispersal(forest, counts, hex, par, float(startFruit-endFruit)/startFruit, indeces);
	phen(forest, hex, par, counts);
	grow(hex, forest, ngaps, urd, par, counts, treesfile);

	// Prune seedling bank
	if(counts.day % (365*10) == 1) prune(forest, par, counts);
}
// END SIMULATION

// OUTPUT SNAPSHOT OF FINAL STATE
// pushBanks(forest, par, true);
pushMemory(foragers, par);
pushFruit(forest, par);
treesfile.close();

return 0;
};

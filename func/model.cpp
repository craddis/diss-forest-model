#include <vector>
#include <random>
#include <deque>
#include <algorithm>
#include <string>
#include "random.hpp"
#include "lattice.hpp"
#include "output.hpp"

namespace params {
	int m; // rows of cells
	int n; // columns of cells
	int N; // initial number of agents in each population
	int lag; // gut retention time (move scale)
	double jspan; // average lifespan of juvenile
	double aspan; // average lifespan of adult
	int baby; // number of steps at finest time scale
	int mama; // number of steps at medium time scale
	int papa; // number of steps at largest time scale
}

struct cell {
	bool resource = false;
	bool propagule = false;
	int visits = 0;
	std::vector<int> adj;
	std::vector<int> bank;
	cell() {bank.push_back(0); bank.push_back(0);}
};

typedef std::vector<cell> landscape;

struct agent {
	int loc = 0;
	std::deque<int> cache;
};

typedef std::vector<agent> population;

bool deposit(agent& A) {
	if(!A.cache.empty()) {
		if(A.cache.front() == 0) { 
			A.cache.pop_front();
			return true;
		}
		else return false;
	}
}

int choose(const std::vector<int>& neighbors, std::function<bool (int)> evaluate) {
	std::vector<int> choices;
	std::copy_if(neighbors.begin(), neighbors.end(), back_inserter(choices), evaluate);
	if(choices.empty()) return *keittlab::one_of(neighbors);
	else return *keittlab::one_of(choices);
}

void move(population& P, landscape& L, bool directed, bool pref) {
	for(auto& A : P) {
		auto evaluate = [&](int i){return L[i].resource == pref && L[i].propagule;};
		if(directed) A.loc = choose(L[A.loc].adj, evaluate);// Choose preferred neighbor if available
		else A.loc = *keittlab::one_of(L[A.loc].adj); // Choose random neighbor
		for(auto& p : A.cache) --p; // decrement cache (1 -> 0)
		if(evaluate(A.loc)) { // for the right kind of seed...	
			A.cache.push_back(params::lag); // Add seed to cache
			L[A.loc].propagule = false; // Remove seed from cell
		}
		if(deposit(A)) ++L[A.loc].bank[pref]; // if ready, deposit seed
	}
} 

void flush(population& P, landscape& L, bool pref) {
	for(auto& A : P) {
		L[A.loc].bank[pref] += A.cache.size();
		A.cache.clear();
	}
}

void phen(landscape& L) {
	for(auto& C : L) {
		if(C.propagule) {
			C.propagule = false;
			C.adj.push_back(-1);
			int dest = *keittlab::one_of(C.adj);
			if(dest == -1) ++C.bank[C.resource];
			else ++L[dest].bank[C.resource];
			C.adj.pop_back();
		}
		C.propagule = true;
	}
}

void grow(landscape& L) {
	for(auto& C : L) {
		// Bank Mortality
		std::binomial_distribution<> death_matrix(C.bank[0], 1.0/params::jspan);
		std::binomial_distribution<> death_focal(C.bank[1], 1.0/params::jspan);
		C.bank[0] -= death_matrix(keittlab::urng());
		C.bank[1] -= death_focal(keittlab::urng());
		// Adult Mortality
		if(keittlab::maybe(1.0/params::aspan)) { // Gap Creation
			if(C.bank[1]){
				auto focal_wins = double(C.bank[1]) / double(C.bank[1] + C.bank[0]);
				C.resource = keittlab::maybe(focal_wins); // Choose Crown
			}
			else C.resource = false;
			C.bank[0] = 0; C.bank[1] = 0; // Reset Bank
		}
	}
}

int main(int argc, char* argv[]) {
std::string run_id = argv[1];
params::m = std::stoi(argv[2]);
params::n = std::stoi(argv[3]);
params::N = std::stoi(argv[4]);
params::lag = std::stoi(argv[5]);
params::aspan = std::stod(argv[6]);
params::jspan = std::stod(argv[7]);
params::baby = std::stoi(argv[8]);
params::mama = std::stoi(argv[9]);
params::papa = std::stoi(argv[10]);

std::ofstream params;
params.open("res/" + run_id + "p", std::ios::app);
params << "m = " << params::m << std::endl;
params << "n = " << params::n << std::endl;
params << "N = " << params::N << std::endl;
params << "lag = " << params::lag << std::endl;
params << "aspan = " << params::aspan << std::endl;
params << "jspan = " << params::jspan << std::endl;
params << "baby = " << params::baby << std::endl;
params << "mama = " << params::mama << std::endl;
params << "papa = " << params::papa << std::endl;
params.close();

keittlab::randomize();
landscape forest(params::m * params::n);
for(int c = 0; c != params::m*params::n; ++c) forest[c].adj = mesh_adj(c, params::m, params::n);
population foragers(params::N);
population wanderers(params::N);
for(auto& C : forest) C.resource = keittlab::maybe(0.5);
for(auto& C : forest) C.propagule = 1;

for(int tg = 0; tg != params::papa; ++tg) {
	for(int tp = 0; tp != params::mama; ++tp) {
		for(int tm = 0; tm != params::baby; ++tm) {
			move(foragers, forest, true, true);
			move(wanderers, forest, false, false);
			push_pop(foragers, tg, run_id, [&](int t){return t > params::papa - 11;});
		}
		flush(foragers, forest, true);
		flush(wanderers, forest, false);
		phen(forest);
	}
	grow(forest);
	push_land(forest, tg, run_id, [&](int t){return t > params::papa - 2;});
}

return 0;
};

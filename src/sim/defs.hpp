// Version 3.5
#include "lattice.hpp"
#include <cmath>
#include <deque>
#include <random>
#include <boost/math/special_functions/beta.hpp> // Boost 1.58

using namespace std;
using namespace keittlab;

int discretePowerLaw(uniform_real_distribution<double>& URD, int MIN, int MAX, double EXP, double C) { // TESTED
	int discrete = MAX+1;
	while( discrete > MAX ) {
		double r = URD(urng());
		double continuous = (MIN-0.5)*pow(1-r, -1.0/(EXP-1)) + 0.5;
		discrete = round(C*continuous);
	}
	return discrete;
}

struct state {
	int tree_id, Nf, day;
	state() :
		tree_id(0), day(0), Nf(0) {}
};

struct parameters {
	int A, years, lag, steps, days, nrot, cr, kmax, Q, R;
	double aspan, jspan, jdays, amort, nu, U, gr, C, beta, beta2, p, L;
	bool sleep, naive, wrap;
	string resdir, rid, land, pop, version;
	parameters(string); // Constructor: defined in io.hpp
};

// LANDSCAPE
int fruitFlux(int size, int nrot) { // TESTED
	return size / nrot + maybe(float(size%nrot)/nrot);
}

struct plant {
	int id; // ID is only assigned when plant reaches adult tree status
	int parent; // ID of parent plant
	bool species; // Focal species is TRUE, matrix species is FALSE
	int size; // How large is this tree (it may not be based on time since emergence)
	int fruit; // How many fruit does the tree have?
	int est; // On what day was this plant established?
	int emrg; // On what day did the plant emerge into the canopy?
	bool disp; // Was this plant dispersed by an agent?
	plant() : // To create empty trees when landscape is created
		id(-1), parent(-1), species(false), size(-1), fruit(0), est(-1), emrg(-1), disp(false) {}
	plant(int parent, bool species, int est, bool disp): // Create new seed during simulation
		id(-1), species(species), parent(parent), size(-1), fruit(0), est(est), emrg(-1), disp(disp) {}
	void emerge(uniform_real_distribution<double>& URD, parameters& PAR, state& S) {
		id = S.tree_id++;
		size = discretePowerLaw(URD, PAR.nrot, PAR.kmax, PAR.L, PAR.C);
		fruit = fruitFlux(size, PAR.nrot);
		emrg = S.day;
	}
};

struct cell {
	int snake = -1; // Position of the cell in the landscape vector
	plant tree; // Each cell has one and only one tree
	deque<plant> bank; // Seed or seedling bank
};

void pushTree(ostream& os, cell& C) {
        os << C.snake <<" "<< C.tree.id <<" "<< C.tree.species <<" "<< C.tree.parent<<" "<< C.tree.emrg << endl;
}

typedef vector<cell> landscape;

void drop(int orig, landscape& L, lattice& H, parameters& PAR, state& S) { // TESTED
	int dest = orig; // Seed will just fall beneath the crown
	if(maybe(PAR.gr)) dest = *one_of(H.adjlist.find(orig)->second); // Unless there is secondary dispersal
	if(maybe(PAR.U)) L[dest].bank.emplace_back(plant(L[orig].tree.id, L[orig].tree.species, S.day, false)); // Or it is destroyed
	--L[orig].tree.fruit; // Remove fruit from tree
}

void rot(cell& C, landscape& L, lattice& H, parameters& PAR, state& S) { // TESTED
	if(C.tree.fruit>C.tree.size) {
		//For each rotten fruit, drop it to the ground
		int rotten = C.tree.fruit-C.tree.size;
		for(int i=0; i!=rotten; ++i) {
			drop(C.snake, L, H, PAR, S);
		}
		assert(C.tree.fruit == C.tree.size); // Only excess fruit should have fallen
	}
}

int totalFruit(landscape& L, bool species) {
	int count = 0;
	for(auto& C : L) {
		if(C.tree.species==species) count += C.tree.fruit;
	}
	return count;
}

// Disperse matrix seeds in equivalence to agent-based focal dispersal for the day
void matrixDispersal(landscape& L, state& S, lattice& H, parameters& PAR, double dispeff, vector<int>& indeces) { // TESTED
	shuffle(indeces.begin(), indeces.end(), urng()); // Shuffle cell order
	int left = round(dispeff * totalFruit(L, false)); // Calculate matrix fruit to disperse
	auto it = indeces.begin();
	int dest = -1;
	while(left > 0) {
		if(L[*it].tree.species == false) {
			if(left >= L[*it].tree.fruit) {
				for(int i=0; i!=L[*it].tree.fruit; ++i) {
					dest = H.rwalk(*it, PAR.lag);
					L[dest].bank.emplace_back(plant(L[*it].tree.id, L[*it].tree.species, S.day, true));
				}
				left -= L[*it].tree.fruit;
				L[*it].tree.fruit = 0;
			}
			else left -= L[*it].tree.fruit;
		}
		++it;
	}
}

void phen(landscape& L, lattice& H, parameters& PAR, state& S) {
	for(auto& C : L) {
		if(C.tree.species) {
			C.tree.fruit += fruitFlux(C.tree.size, PAR.nrot); // Add new fruit
			rot(C, L, H, PAR, S); // Rotten fruit fall from tree
		}
		else {
			C.tree.fruit += fruitFlux(C.tree.size, PAR.nrot); // Add new fruit
			rot(C, L, H, PAR, S); // Rotten fruit fall from tree
		}
	}
}

void grow(lattice& H, landscape& L, binomial_distribution<>& BD, uniform_real_distribution<double> URD, parameters& PAR, state& S, ostream& F) {
	auto dead = [&](plant& p){return p.est <= (S.day - PAR.jdays);};
	int ngaps = BD(urng());
	assert( ngaps >= 0 );
	double densDepend = 0.5;
	if(ngaps > 0) densDepend = boost::math::ibetac(PAR.beta, PAR.beta2, S.Nf*1.0/H.N);
	for(int i=0; i < ngaps; ++i) { // How many gaps are created today?
		auto& C = *one_of(L); // Refer to a random cell
		auto firstDead = find_if(C.bank.rbegin(), C.bank.rend(), dead); // Find first dead seedling by searching from back to front
		if(firstDead == C.bank.rbegin()) { // If seedling bank is empty (or has no live seeds)...
			//cout<<"No live seeds"<<endl;
			// Replace old tree with new matrix tree
			S.Nf -= C.tree.species;
			C.tree = plant(-2, false, -1, false);
			C.tree.emerge(URD, PAR, S);
		}
		else {
			//cout<<"Live seeds"<<endl;
			int nseeds = distance(C.bank.rbegin(), firstDead);
			auto firstFalse = partition(C.bank.rbegin(), firstDead, [](plant& p){return p.species;});
			int nTrue = distance(C.bank.rbegin(), firstFalse);
			double pTrue = nTrue*1.0/nseeds;
			double ppTrue = pTrue*densDepend / (pTrue*densDepend + (1-densDepend)*(1-pTrue));
			deque<plant>::reverse_iterator winner;
			if(maybe(ppTrue)) winner = one_of_range(C.bank.rbegin(), firstFalse); // Point to winner
			else winner = one_of_range(firstFalse, firstDead);
			S.Nf += (winner->species - C.tree.species); // Update focal tree count
			C.tree = *winner; // Replace old tree with new one
			C.tree.emerge(URD, PAR, S);
			C.bank.clear(); // Clear seedling bank
		}
		pushTree(F, C);
	}
}

// Remove dead seeds/seedlings from bank to free up memory
void prune(landscape& L, parameters& PAR, state& S) { // UNCHANGED FROM SEED 2.4
	auto alive = [&](plant& p){return p.est > (S.day - PAR.jdays);}; // True when seedling is alive
	for(auto& C : L) { // For each cell on the landscape...
		auto firstLiving = find_if(C.bank.begin(), C.bank.end(), alive); // Find the first live seedling...
		C.bank.erase(C.bank.begin(), firstLiving); // And erase all seedlings up to, but not including, this first living one
	}
}

// MOVEMENT
struct agent {
	int id = -1;
	int snake = -1;
	deque<pair<int,int>> cache;
	unordered_map<int, int> memory;
};

typedef vector<agent> population;

/**void predict(agent& A, landscape& L) {
	for(auto& place : A.memory) {
		place.second = L[place.first].tree.fruit;
	}
}**/

void predict(agent& A, landscape& L, parameters& PAR) {
	for(auto& loc : A.memory) {
		loc.second += fruitFlux(L[loc.first].tree.size, PAR.nrot);
		if(loc.second>L[loc.first].tree.size) loc.second=L[loc.first].tree.size;
	}
}

double getValue(double nfruit, double distance) { // TESTED
	return nfruit / distance;
}

void eat(agent& A, cell& C, parameters& PAR) { // TESTED
	if(PAR.cr >= C.tree.fruit) { // If the agent's appetite is greater than the fruit on the tree...
		for(int i=0; i!=C.tree.fruit; ++i) {
			A.cache.emplace_back(make_pair(PAR.lag+1, C.tree.id)); // Swallow seed
		}
		C.tree.fruit = 0; // All the fruit has been eaten
	}
	else {
		for(int i=0; i!= PAR.cr; ++i) {
			--C.tree.fruit; // Remove fruit from tree
			A.cache.emplace_back(make_pair(PAR.lag+1, C.tree.id)); // Swallow seed
		}
	}
}

int best(agent& A, lattice& H, parameters& PAR) {
	vector<int> finalists;
	int maximum = -1;
	int value = -1;
	for(auto& j : A.memory) {
		value = getValue(j.second, H.getNYC(A.snake, j.first));
		if(value > maximum) { // Is that value the highest one so far?
			maximum = value; // If so, reset the maximum
			finalists.clear();
			finalists.push_back(j.first);
		}
		else if(value == maximum) finalists.push_back(j.first);
	}
	if(finalists.empty()) return -1;
	else return *one_of(finalists);
}

int move(agent& A, lattice& H, parameters& PAR) {
	int ideal = best(A, H, PAR);
	double p;
	if(ideal == -1) p = 1;
	else {
		double e = getValue(A.memory[ideal], H.getNYC(A.snake, ideal));
		p = exp(-1*e / PAR.nu);
	}
	if(maybe(p)) { // Agents takes random step
		A.snake = *one_of(H.adjlist.find(A.snake)->second);
		return -1;
	}
	else {
		A.snake = H.direction(A.snake, ideal);
		return ideal;
	}
}

int takeStep(agent& A, landscape& L, lattice& H, parameters& PAR, state& S) {
	cell& C = L[A.snake];
	//DIGEST
	for(auto& s : A.cache) --s.first; // decrement cache (1 -> 0)
	//DEPOSIT
	auto notready = [](pair<int, int>& s){return s.first>0;}; //
	auto firstnotready = find_if(A.cache.begin(), A.cache.end(), notready); //
	for(auto it=A.cache.begin(); it!=firstnotready; ++it) {
		C.bank.emplace_back(plant(it->second, true, S.day, true));
	}
	A.cache.erase(A.cache.begin(), firstnotready); //
	// DECIDE:
	if(C.tree.species) { // If the cell contains a focal tree...
		if(C.tree.fruit>0) { // And there is fruit...
			eat(A, C, PAR); // Eat the fruit, and stay
			return -2; // indicates eating
		}
		else { // But if the focal tree is empty...
			A.memory[A.snake] = 0; // Remember an empty focal tree...
			return move(A, H, PAR); // And then leave
		}
	}
	else { // But if the cell contains a matrix tree
		if(A.memory.count(A.snake)>0) { // And you remember a focal tree here...
			A.memory.erase(A.snake); // Then forget that focal tree
		}
		return move(A, H, PAR); // Then leave either way
	}
}

void seedflush(agent& A, landscape& L, state& S) {
	if(!A.cache.empty()) {
		for(auto& s : A.cache) L[A.snake].bank.emplace_back(plant(s.second, true, S.day, true));
		A.cache.clear();
	}
}

// INITIALIZATION FUNCTIONS
void InitLandRand(lattice& H, landscape& L, parameters& PAR, state& S, uniform_real_distribution<double>& URD, ostream& F) {
	for(int i=0; i!=H.N; ++i) { // Loop through cells
		L[i].snake=i; // Cell ID = vector position
		L[i].tree.id = S.tree_id++; // Tree IDs starting from 0
		L[i].tree.species = maybe(PAR.p); // Set species with Bernoulli trial
		L[i].tree.size = discretePowerLaw(URD, PAR.nrot, PAR.kmax, PAR.L, PAR.C);
		L[i].tree.fruit = fruitFlux(L[i].tree.size, PAR.nrot);
		S.Nf += L[i].tree.species;
		pushTree(F, L[i]);
	}
}

void InitPopRand(lattice& H, population& P) {
	int id_counter = 0;
	for(auto& A : P) { // Loop through population
		A.id = id_counter++;
		A.snake = pick_a_number(0, H.N-1);
	}
}

void inform(landscape& L, agent& A, parameters& PAR) { // TESTED
	for(auto& C : L) {
		if(C.tree.species) A.memory[C.snake]=C.tree.fruit;
	}
}

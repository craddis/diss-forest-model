// Version 2.3.1
//using namespace std;
//using namespace keittlab;

struct state_var {
	int tree_id, Nfd, Nf, switches, local;
	double Ed;
	void update() {
		Ed = 1.0*Nfd / Nf;
	}
	state_var() :
		tree_id(0), Nfd(0), Nf(0), switches(0), local(0), Ed(0) {}
};

struct parameters {
	int A, lag, steps, days;
	double jdays, amort, tol, a, E, f, U, gr;
	string resdir, rid;
	parameters(int A, int lag, double jspan, double aspan, int steps, int years, int theta, double a, double E, double f, double U, double gr, string resdir, string rid) :
		A(A), 
		lag(lag), 
		steps(steps), 
		jdays(jspan*365),
		amort(1 / aspan / 365),
		days(years*365),
		a(a),
		tol((1-a)*pow(a, theta)),
		E(E),
		f(f),
		U(U),
		gr(gr),
		resdir(resdir),
		rid(rid) {} 
};

struct plant { // Includes seeds, seedlings and trees
	int id; // ID is only assigned when plant reaches adult tree status
	int parent; // ID of parent plant
	bool species; // Focal species is TRUE, matrix species is FALSE 
	bool fruit; // Does the plant have fruit?
	int est; // On what day was this plant established?
	plant() : // To create empty trees when landscape is created
		id(-1), parent(-1), species(false), fruit(false), est(-1) {}
	plant(int& tree_id) : // To create new trees when bank is empty
		id(tree_id++), parent(-1), species(false), fruit(false), est(-1) {}
	plant(bool x, int y, int e) : // To create seeds when fruit is eaten or dropped
		id(-1), parent(y), species(x), fruit(false), est(e) {}
};

struct cell {
	int snake = -1; // Position of the cell in the landscape vector
	plant tree; // Each cell has one and only one tree
	deque<plant> bank; // Seed or seedling bank
};

typedef vector<cell> landscape;

struct agent {
	int snake; // Where the agent is
	double trees; // Decaying memory of TRUE trees encountered
	double rewards; // Decaying memory of fruit encountered
	bool local;
	int dest;
	deque< pair<int, plant> > cache; // The agent's gut through which seeds pass
	vector<double> longM; // Long term memory of the value of each cell in the landscape
	unordered_set<int> shortM; // History of cells visited today
	agent() {
		snake = -1; 
		trees = 0;
		rewards = 0;
		local = true;
		dest = -1;
	}
};

typedef vector<agent> population;


int best(agent& A, lattice& H, double E) {
	vector<int> finalists;
	double maximum = -H.N;
	double value;
	for(int i=0; i!=H.N; ++i) {
		if(A.shortM.count(i)==0) { // Have you already visited there today?
			value = E * A.longM[i] - H.manhattan(A.snake, i); // If not, then calculate the value of the cell
			if(value > maximum) { // Is that value the highest one so far?
				maximum = value; // If so, reset the maximum
				finalists.clear();
				finalists.push_back(i);
			}
			else if(value == maximum) finalists.push_back(i);
		}
	}
	assert(!finalists.empty());
	return *one_of(finalists);
}

void BiModalStep(agent& A, landscape& L, lattice& H, parameters& PAR, state_var& SV, int day) {
	//OBSERVE
	auto edible = [&](int i){return L[i].tree.species && L[i].tree.fruit;}; // Define functor to evaluate cells
	A.trees = A.trees*PAR.a + L[A.snake].tree.species*(1-PAR.a); // Update tree record
	//EAT
	if(edible(A.snake)) { // Does the tree have edible fruit?
		L[A.snake].tree.fruit = false; // If so, remove fruit from tree
		A.cache.emplace_back(make_pair(PAR.lag + 1 , plant(true, L[A.snake].tree.id, day))); // Add seed to cache
		A.rewards = A.rewards*PAR.a + (1-PAR.a); // Update reward record
		++SV.Nfd; // Add to counter of Focal seeds dispersed
	}
	else A.rewards = A.rewards*PAR.a; // Update reward record
	//REMEMBER
	A.shortM.insert(A.snake); // Record cell visit
	A.longM[A.snake] = A.trees; // Commit record of trees to long term memory
	//DIGEST
	for(auto& p : A.cache) --p.first; // decrement cache (1 -> 0)
	//DEPOSIT
	if(!A.cache.empty() && A.cache.front().first == 0) {// If there is a seed ready...
		L[A.snake].bank.push_back(A.cache.front().second); // Add seed to bank
		A.cache.pop_front(); // Remove seed from gut
	}
	//MOVE
	vector<int>& neighbors = H.adjlist.find(A.snake)->second;
	vector<int> edibles;
	copy_if(neighbors.begin(), neighbors.end(), back_inserter(edibles), edible);
	if(!edibles.empty()) { // If there is an edible neighbor, always go there
		++SV.local; SV.switches += (true - A.local); // Track movement states
		A.local = true; 
		A.dest = -1;
		A.snake = *one_of(edibles);
		//cout<<"door 1"<<endl;
	} 
	else if(A.rewards >= PAR.tol || A.snake == A.dest) { // If not, but the patch has been good, pick a random neighbor
		++SV.local; SV.switches += (true - A.local); // Track movement states
		A.local = true;
		A.dest = -1;
		A.snake = *one_of(neighbors);
		//cout<<"door 2"<<endl;
	}
	else {
		if(A.local) {
			A.dest = best(A, H, PAR.E);
			++SV.switches; // Track movement states
			//cout<<"door 3"<<endl;
		}
		A.local = false;
		A.snake = H.direction(A.snake, A.dest); // Otherwise take a step towards the best global location
	}
}

void seedflush(agent& A, landscape& L) {
	if(!A.cache.empty()) {
		for(auto& p : A.cache) L[A.snake].bank.push_back(p.second);
		A.cache.clear();
	}
}

void memflush(agent& A) {
	A.rewards = 0;
	A.shortM.clear();
}

void prune(landscape& L, parameters& PAR, state_var& SV, int day) {
	auto alive = [&](plant& p){return p.est > (day - PAR.jdays);}; // True when seedling is alive
	for(auto& C : L) {
		auto firstLiving = find_if(C.bank.begin(), C.bank.end(), alive);
		C.bank.erase(C.bank.begin(), firstLiving); // Erase all seedlings up to, but not including, the first living one
	}
}

void phen(landscape& L, lattice& H, parameters& PAR, state_var& SV, int day) {
	int dest = -1;
	for(auto& C : L) {
		if(C.tree.fruit) { // Does it still have fruit?
			if(C.tree.species==false && maybe(SV.Ed)) {
				dest = H.rwalk(C.snake, PAR.lag);
				L[dest].bank.emplace_back(plant(C.tree.species, C.tree.id, day));
			}
			else {
				if(maybe(PAR.gr)) dest = *one_of( H.adjlist.find(C.snake)->second ); // Choose cell for seed to disperse to
				else dest = C.snake;
				if(maybe(PAR.U)) L[dest].bank.emplace_back(plant(C.tree.species, C.tree.id, day));
			}
		}
		C.tree.fruit = maybe(PAR.f); // with probability f, give tree fruit
	}
	SV.Nfd = 0; // Reset counter of focal seeds dispersed in the preceeding day
}

void grow(landscape& L, binomial_distribution<>& ngaps, parameters& PAR, state_var& SV, int day) {
	auto dead = [&](plant& p){return p.est <= (day - PAR.jdays);};
	int NGAPS = ngaps(urng());
	assert( NGAPS >= 0 );
	for(int i=0; i < NGAPS; ++i) { // How many gaps are created today?
		auto& C = *one_of(L); // Refer to a random cell
		auto rit = find_if(C.bank.rbegin(), C.bank.rend(), dead); // Find first dead seedling by searching from back to front
		if(rit == C.bank.rbegin()) { // If seedling bank is empty (or has no live seeds), always grow a matrix tree
			//cout<<"Door 1"<<endl;
			SV.Nf -= C.tree.species; // Update focal tree count
			C.tree = plant(SV.tree_id); // Replace old tree with new matrix tree
			C.tree.fruit = maybe(PAR.f); // Give new tree fruit, with probability f
			assert( C.tree.species == false );
		}
		else {
			//cout<<"Door 2"<<endl;
			auto winner = one_of_range(C.bank.rbegin(), rit); // Point to winner		
			SV.Nf += (winner->species - C.tree.species); // Update focal tree count
			C.tree = *winner; // Replace old tree with new one
			C.bank.clear(); // Clear seedling bank
			C.tree.id = SV.tree_id++; // Give new tree an ID number
			C.tree.fruit = maybe(PAR.f); // Give new tree fruit, with probability f
		}
	}
}

void InitLandRand(lattice& H, landscape& L, double p, state_var& SV) {
	for(int i=0; i!=H.N; ++i) { // Loop through cells
		L[i].snake=i; // Cell ID = vector position
		L[i].tree.id = SV.tree_id++; // Tree IDs starting from 0
		L[i].tree.species = maybe(p); // Set species with Bernoulli trial
		SV.Nf += L[i].tree.species; // Update focal tree counter
		L[i].tree.fruit = true; // All trees start with fruit
	}
}

void InitPopRand(lattice& H, population& P) {
	for(auto& A : P) { // Loop through both populations
		A.snake = pick_a_number(0, H.N-1);
		for(int i=0; i!=H.N; ++i) A.longM.push_back(0);
	}
}

void loadLand(lattice& H, landscape& L, std::string filename, state_var& SV) {
	for(int i=0; i!=H.N; ++i) { // Loop through cells
		L[i].snake=i; // Cell ID = vector position
		L[i].tree.fruit=true; // All trees start with fruit
	}
	ifstream landfile(filename);
	string line; vector<int> temp;	
	while(getline(landfile, line)) {
		istringstream iss(line);
		for(int i; iss >> i; ) temp.push_back(i);
		if(temp[1] == -1) {
			L[temp[0]].bank.emplace_back(plant(temp[2], temp[3], temp[4]));
		}
		else {
			L[temp[0]].tree.id=temp[1]; 
			L[temp[0]].tree.species=temp[2];
			L[temp[0]].tree.parent=temp[3];
			L[temp[0]].tree.est=temp[4];
		}
		temp.clear();
	}
	landfile.close();
	auto compfn = [&](cell i, cell j){return i.tree.id < j.tree.id;};
	SV.tree_id = max_element(L.begin(), L.end(), compfn)->tree.id;
	++SV.tree_id;
}

void loadPop(lattice& H, population& P, std::string filename) {
	ifstream popfile(filename);
	string line; vector<int> temp;
	int agent_id = 0;
	while(getline(popfile, line)) {
		istringstream iss(line);
		for(int i; iss >> i; ) temp.push_back(i);
		P[agent_id].snake = temp[0];
		P[agent_id].trees = temp[1];
		for(int j=2; j!=2+H.N; ++j) P[agent_id].longM.push_back(temp[j]); 		
		temp.clear();
		++agent_id;
	}
	popfile.close();
}

void inform(landscape& L, population& P, parameters& PAR) {
	for(auto& A : P) {
		for(auto& C : L) {
			if(C.tree.species) A.longM[C.snake] = 1-PAR.a;
		}
	}
}

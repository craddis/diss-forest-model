// Version 3.5
#include "defs.hpp"
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <boost/algorithm/string.hpp>

parameters::parameters(string RID) {
  ifstream parfile("/home/colin/projects/SEED/init/params."+RID);
  string line;
  while(getline(parfile, line)) {
    int eqpos = line.find("=");
    string parname = line.substr(0, eqpos);
    string value = line.substr(eqpos+1);
    if(parname=="rid") rid = value;
    else if(parname=="years") years = stoi(value);
    else if(parname=="steps") steps = stoi(value);
    else if(parname=="version") version = value.substr(1, value.size()-2);
    else if(parname=="A") A = stoi(value);
    else if(parname=="cr") cr = stoi(value);
    else if(parname=="lag") lag = stoi(value);
    else if(parname=="nu") nu = stod(value);
    else if(parname=="sleep") sleep = bool(stoi(value));
    else if(parname=="naive") naive = bool(stoi(value));
    else if(parname=="pop") pop = value.substr(1, value.size()-2);
    else if(parname=="aspan") aspan = stod(value);
    else if(parname=="jspan") jspan = stod(value);
    else if(parname=="nrot") nrot = stoi(value);
    else if(parname=="gr") gr = stod(value);
    else if(parname=="U") U = stod(value);
    else if(parname=="C") C = stod(value);
    else if(parname=="Q") Q = stoi(value);
    else if(parname=="R") R = stoi(value);
    else if(parname=="L") L = stod(value);
    else if(parname=="p") p = stod(value);
    else if(parname=="kmax") kmax = stoi(value);
    else if(parname=="beta") beta = stod(value);
    else if(parname=="land") land = value.substr(1, value.size()-2);
    else if(parname=="wrap") wrap = bool(stoi(value));
    else if(parname=="resdir") resdir = value.substr(1, value.size()-2);
    else {
      cout << "Invalid Input Parameter" << endl;
      exit (EXIT_FAILURE);
    }
  }
  jdays = jspan*365;
  amort = 1 / aspan / 365;
  days = years*365;
  beta2 = (1-p) * beta / p;
}

std::ostream& operator<<(std::ostream& os, const parameters& PAR) {
  os << "rid=" << PAR.rid << endl;
  os << "years=" << PAR.years << endl;
  os << "steps=" << PAR.steps << endl;
  os << "version=" << PAR.version << endl;
  os << "A=" << PAR.A << endl;
  os << "cr=" << PAR.cr << endl;
  os << "lag=" << PAR.lag << endl;
  os << "nu=" << PAR.nu << endl;
  os << "sleep=" << PAR.sleep << endl;
  os << "naive=" << PAR.naive << endl;
  os << "pop=" << PAR.pop << endl;
  os << "aspan=" << PAR.aspan << endl;
  os << "jspan=" << PAR.jspan << endl;
  os << "nrot=" << PAR.nrot << endl;
  os << "gr=" << PAR.gr << endl;
  os << "U=" << PAR.U << endl;
  os << "C=" << PAR.C << endl;
  os << "Q=" << PAR.Q << endl;
  os << "R=" << PAR.R << endl;
  os << "L=" << PAR.L << endl;
  os << "p=" << PAR.p << endl;
  os << "kmax=" << PAR.kmax << endl;
  os << "beta=" << PAR.beta << endl;
  os << "land=" << PAR.land << endl;
  os << "wrap=" << PAR.wrap << endl;
  os << "resdir=" << PAR.resdir << endl;
  os << "jdays=" << PAR.jdays << endl;
  os << "amort=" << PAR.amort << endl;
  os << "days=" << PAR.days << endl;
  os << "beta2=" << PAR.beta2;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    copy(v.begin(), v.end(), std::ostream_iterator<T>(os," "));
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::deque<T>& d) {
    copy(d.begin(), d.end(), std::ostream_iterator<T>(os," "));
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& us) {
    copy(us.begin(), us.end(), std::ostream_iterator<T>(os," "));
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<int, T>& um) {
    for(auto& i : um) os << i.first<<":"<< i.second <<" ";
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::array<T, 2> a) {
  os << a[0] << " " << a[1];
  return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::array<T, 3> a) {
  os << a[0] << " " << a[1] << " " << a[2];
  return os;
}

void pushBanks(landscape& L, parameters& PAR, bool SP) {
	ofstream file(PAR.resdir+"/banks."+PAR.rid);
	for(auto& C : L) {
		unordered_map<int,int> pool;
		for(auto& S : C.bank) {
			if(S.species==SP) ++pool[S.parent];
		}
		for(auto it=pool.begin(); it!=pool.end(); ++it) {
			file << C.snake<<" "<< it->first<<" "<< it->second << endl;
		}
	}
	file.close();
}

void pushMemory(population& P, parameters& PAR) {
  // SUCCESSFUL TEST 2015-12-01
	ofstream file(PAR.resdir+"/memory."+PAR.rid);
	for(auto& A : P) file << A.memory << endl;
	file.close();
}

void pushFruit(landscape& L, parameters& PAR) {
  // SUCCESSFUL TEST 2015-12-01
	ofstream file(PAR.resdir+"/fruit."+PAR.rid);
	for(auto& C : L) {
		file << C.snake <<" "<< C.tree.id <<" "<< C.tree.species <<" "<< C.tree.size<<" "<< C.tree.fruit << endl;
	}
	file.close();
}

// INPUT
void loadMemory(population& P, parameters& PAR) {
  // SUCCESSFUL TEST 2015-12-01
  for(auto& A : P) A.memory.clear(); // Clear memories, if there are any
  ifstream file(PAR.resdir+"/memory."+PAR.rid);
	string line;
	int Aid=0;
	while(getline(file, line)) {
		istringstream iss(line);
		for(string i; iss >> i; ) {
			int delim = i.find(":");
			int snake = stoi(i.substr(0, delim));
			int fruit = stoi(i.substr(delim+1));
			P[Aid].memory[snake] = fruit;
		}
		++Aid;
	}
  file.close();
}

void loadFruit(landscape& L, parameters& PAR) {
  // SUCCESSFUL TEST 2015-12-01
  ifstream file(PAR.resdir+"/fruit."+PAR.rid);
  string line; vector<int> temp;
  while(getline(file, line)) {
    istringstream iss(line);
    for(int i; iss >> i; ) temp.push_back(i);
    L[temp[0]].snake = temp[0];
    L[temp[0]].tree.id = temp[1];
    L[temp[0]].tree.species = temp[2];
    L[temp[0]].tree.size = temp[3];
    L[temp[0]].tree.fruit = temp[4];
    temp.clear();
  }
}

unordered_set<int> getEndCanopy(parameters& PAR) {
	unordered_set<int> canopy;
	ifstream file(PAR.resdir+"/fruit."+PAR.rid);
	string line; vector<int> temp;
	while(getline(file, line)) {
		istringstream iss(line);
		for(int i; iss >> i; ) temp.push_back(i);
		if(temp[2]==1) canopy.insert(temp[0]);
		temp.clear();
	}
	return canopy;
}

vector<unordered_set<int>> getAllCanopy(parameters& PAR) {
	vector<unordered_set<int>> output;
	ifstream file(PAR.resdir+"/trees."+PAR.rid);
	string line; vector<int> temp;
	int current_year = 0;
	unordered_set<int> canopy;
	while(getline(file, line)) {
		istringstream iss (line);
		for(int i; iss >> i; ) temp.push_back(i);
		int emrg_year = temp[4] / 365 + (temp[4]>=0);
		if(emrg_year > current_year) {
			output.push_back(canopy); // save snapshot of canopy
			current_year = emrg_year; // advance the clock
		}
		//  Either way, now emrg_year = current_year 
		// b/c temp[4] is chronological
		// If focal, insert the tree
		if(temp[2]==1) canopy.insert(temp[0]);
		// if the tree is already there, nothing happens
		// Otherwise, it's matrix, so erase the tree
		else canopy.erase(temp[0]);
		// if it wasn't there to being with, nothing happens
		temp.clear();
	}
	output.push_back(canopy); // push endpoint of canopy
	return output;
}

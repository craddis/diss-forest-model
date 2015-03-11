//#include "boost/program_options.hpp"
//#include "boost/any.hpp"
//#include <iostream>
//#include <fstream>

template <typename pseudomap>
void push_params(const pseudomap& vm, std::string rid) {
	std::ofstream file ("params."+rid, std::ios::app);
	for(auto& V : vm) {
		if(typeid(int) == V.second.value().type()) file << V.first << "="  << boost::any_cast<int>(V.second.value()) << ";" << std::endl;
		else if(typeid(double) == V.second.value().type()) {
			file << V.first << "="  << boost::any_cast<double>(V.second.value()) << ";" << std::endl;}
		else if(typeid(std::string) == V.second.value().type()) {
			file << V.first << "='"<< boost::any_cast<std::string>(V.second.value()) << "';" << std::endl;}
	}
	file.close();
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
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& v) {
    copy(v.begin(), v.end(), std::ostream_iterator<T>(os," "));
    return os;
}

void push_state(landscape& L, population& P, parameters& PAR, std::string rid) {
	auto older = [&](plant& p){return p.est < (PAR.days - PAR.jdays);};
	std::ofstream landfile("land.state."+rid);
	for(auto& C : L) {
		landfile << C.snake <<" "<< C.tree.id <<" "<< C.tree.species <<" "<< C.tree.parent <<" "<< C.tree.est << endl;
		auto rit = find_if(C.bank.rbegin(), C.bank.rend(), older);
		for(auto P = C.bank.rbegin(); P!=rit; ++P) {
			landfile << C.snake <<" "<< P->id <<" "<< P->species <<" "<< P->parent <<" "<< P->est << endl;
		}
	}
	landfile.close();
	std::ofstream popfile("pop.state."+rid);
	for(auto& A : P) {
		popfile << A.snake <<" "<< A.trees <<" "<< A.longM << A.shortM << endl;
	}
	popfile.close();
}

/** DISPLAY FORMAT FOR TESTING
void displayAgent(agent& A) {
	cout<< "snake: " << A.snake << endl;
	cout<< "gut: ";
	for(auto& p : A.cache) cout << p.first<<"|"<< p.second.species <<"|"<< p.second.parent<<", ";
	cout << endl;
	cout<< "trees:   " << A.trees << endl;
	cout<< "rewards: " << A.rewards << endl;
	cout<< "shortM: " << A.shortM << endl;
	cout<< "longM:  " << A.longM << endl;
}
	
void displayCell(cell& C) {
	cout << "snake: " << C.snake << endl;
	cout << "  bank: ";
	for(auto& P : C.bank) cout << P.species<<"|"<<P.parent;
	cout<< endl << "tree: " << C.tree.id << endl;
	cout << "  species: " << C.tree.species << endl;
	cout << "  fruit: " << C.tree.fruit << endl;
	cout << "  parent: " << C.tree.parent << endl;
	cout << "__________________________" << endl;
}**/

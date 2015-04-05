#include <vector>
#include <random>
#include <deque>
#include <algorithm>
#include <numeric>
#include <string>
#include <boost/program_options.hpp>
#include <utility>
#include <fstream>
#include <iostream>
#include <boost/any.hpp>
#include <cmath>
#include "random.hpp"
#include <unordered_set>
#include <unordered_map>
#include <cassert>

using namespace keittlab;
using namespace std;

#include "lattice.hpp" //requires: <vector> <string> <armadillo>
#include "defs.hpp"
#include "output.hpp"
#include "stats.hpp" // requires cmath, lattice

int main(int argc, char* argv[]) {
// COMMAND LINE OPTIONS CODE
namespace po = boost::program_options;
po::options_description desc("Allowed Options");
desc.add_options()
	("version", po::value<std::string>()->default_value("2.3.1"), "Model Version")
	("rid", po::value<std::string>()->default_value(""), "Simulation Run ID number")	
	("resdir", po::value<std::string>()->default_value("."), "Directory for output files")
    ("R", po::value<int>()->default_value(50), "Nrow in grid")
    ("Q", po::value<int>()->default_value(50), "Ncol in grid")
    ("A", po::value<int>()->default_value(20), "Number of foragers")
    ("lag", po::value<int>()->default_value(5), "Gut retention time")
    ("jspan", po::value<double>()->default_value(2), "Lifspan of advanced regeneration")
    ("aspan", po::value<double>()->default_value(10), "Lifespan of adult trees")
    ("steps", po::value<int>()->default_value(100), "Steps in a day")
    ("years", po::value<int>()->default_value(500), "Years in simulation")
	("land", po::value<std::string>()->default_value("random"), "Starting point for landscape")
	("pop", po::value<std::string>()->default_value("random"), "Starting point for agents")
	("p", po::value<double>()->default_value(0.20), "Initial proportion of focal trees")
	("theta", po::value<int>()->default_value(5), "Minumum patch size")
	("a", po::value<double>()->default_value(0.5), "Memory Retention")
	("E", po::value<double>()->default_value(50), "Resource Conversion Efficiency")
	("f", po::value<double>()->default_value(1), "Proportion of trees that fruit each day")
	("U", po::value<double>()->default_value(1), "Proportion of undispersed seeds that become established")
	("gr", po::value<double>()->default_value(0.2), "Proportion of remainings seeds that are ground dispersed to immediate neighbor")
;

po::variables_map params;
po::store(po::parse_command_line(argc, argv, desc), params);
po::notify(params);

std::string landfile = params["land"].as<std::string>();
std::string popfile = params["pop"].as<std::string>();
double p = params["p"].as<double>();

parameters par(
	params["A"].as<int>(),
	params["lag"].as<int>(),
	params["jspan"].as<double>(),
	params["aspan"].as<double>(),
	params["steps"].as<int>(),
	params["years"].as<int>(),
	params["theta"].as<int>(),
	params["a"].as<double>(),
	params["E"].as<double>(),
	params["f"].as<double>(),
	params["U"].as<double>(),
	params["gr"].as<double>(),
	params["resdir"].as<string>(),
	params["rid"].as<string>()	
);

push_params(params, par);

//MODEL INITIALIZATION
randomize();
lattice hex(params["R"].as<int>(), params["Q"].as<int>());
hex.fillAdj();
binomial_distribution<> ngaps(hex.N, par.amort);
state_var state;
landscape forest(hex.N); // Vector of empty cells/trees objects
population foragers(par.A);

if(landfile=="random") InitLandRand(hex, forest, p, state); // Place trees randomly
else loadLand(hex, forest, landfile, state);

if(popfile=="random") InitPopRand(hex, foragers); // Place agents randomly
else loadPop(hex, foragers, popfile);

if(landfile=="random" && popfile=="random") inform(forest, foragers, par);


//std::ofstream dmfile("daily");
std::ofstream amfile(par.resdir+"/annual."+par.rid);
std::ofstream canopyfile(par.resdir+"/canopy."+par.rid);

int checkNf = 0;

//SIMULATION
for(int day = 0; day != par.days; ++day) {
	for(auto& A: foragers) memflush(A); // Erase short term memory from day before
	state.switches=0; state.local=0; // Reset yesterday's search state counters 
	shuffle(foragers.begin(), foragers.end(), urng());
	for(int step = 0; step != par.steps; ++step) {
		// TIME STEP
		for(auto& A : foragers) BiModalStep(A, forest, hex, par, state, day);
	}
	//END OF THE DAY
	for(auto& A: foragers) seedflush(A, forest); // Void all seeds at sleeping site 
	state.update();
	// DAILY METRICS
	/**/
	phen(forest, hex, par, state, day);
	grow(forest, ngaps, par, state, day);	

	if(state.Nf == 0) {
		checkNf = 0;
		for(auto& C : forest) checkNf += C.tree.species;
		if(checkNf != state.Nf) {
			cout << par.rid << " : Abundance correction : state.Nf=" << state.Nf << ", checkNf=" << checkNf << " on day " << day << endl;
			state.Nf = checkNf;
		}
		else {
			cout << par.rid << " : Exit due to zero abudance, day " << day << endl;
			amfile<< day <<" "<< state.Ed <<" "<< state.Nf <<" ";
			amfile<< state.switches <<" "<< state.local << endl;
			push_state(forest, foragers, par);
			amfile.close();
			canopyfile.close();
			return 0;
		}
	}
	// YEARLY METRICS 
	if(day % 365 == 0) {
		amfile<< day <<" "<< state.Ed <<" "<< state.Nf <<" ";
		amfile<< state.switches <<" "<< state.local << endl;
		for(auto& C : forest) {
			canopyfile << C.tree.species;
		}
		canopyfile << endl;
	}
	
	// Prune seedling bank
	if(day % (365*100) == 1) prune(forest, par, state, day);
}

// ENDPOINT METRICS
/**/
// STATE DUMP
push_state(forest, foragers, par);

// Close files
//dmfile.close();
amfile.close();
canopyfile.close();

return 0;
};

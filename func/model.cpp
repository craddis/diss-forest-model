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
#include <armadillo>
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
	("version", po::value<std::string>()->default_value("2.2"), "Model Version")
	("rid", po::value<std::string>()->default_value(""), "Simulation Run ID number")
    ("R", po::value<int>()->default_value(50), "Nrow in grid")
    ("Q", po::value<int>()->default_value(50), "Ncol in grid")
    ("A", po::value<int>()->default_value(20), "Number of foragers")
    ("lag", po::value<int>()->default_value(5), "Gut retention time")
    ("jspan", po::value<double>()->default_value(2), "Lifspan of advanced regeneration")
    ("aspan", po::value<double>()->default_value(10), "Lifespan of adult trees")
    ("steps", po::value<int>()->default_value(100), "Steps in a day")
    ("years", po::value<int>()->default_value(500), "Years in simulation")
	("init", po::value<std::string>()->default_value("random"), "Starting point for landscape and agents")
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

std::string rid = params["rid"].as<std::string>();
push_params(params, rid);

std::string init = params["init"].as<std::string>();
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
	params["gr"].as<double>()
);

//MODEL INITIALIZATION
randomize();
lattice hex(params["R"].as<int>(), params["Q"].as<int>());
hex.fillAdj();
binomial_distribution<> ngaps(hex.N, par.amort);
state_var state;
landscape forest(hex.N); // Vector of empty cells/trees objects
population foragers(par.A);

InitLandRand(hex, forest, p, state); // Place trees randomly
InitPopRand(hex, foragers); // Place agents randomly

//std::ofstream dmfile("daily");
std::ofstream amfile("annual."+rid);

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
	if(state.Nf==0) {
		push_state(forest, foragers, par, rid);
		amfile.close();
		return 0;
	}
	// DAILY METRICS
	/**/
	phen(forest, hex, par, state, day);
	grow(forest, ngaps, par, state, day);
	// YEARLY METRICS 
	if(day % 365 == 0) {
		amfile<< day <<" "<< state.Ed <<" "<< abundance(hex, state) <<" ";
		amfile<< state.switches <<" "<< state.local << endl;
	}
}

// ENDPOINT METRICS
/**/
// STATE DUMP
push_state(forest, foragers, par, rid);

// Close files
//dmfile.close();
amfile.close();

return 0;
};

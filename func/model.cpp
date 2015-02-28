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
	("version", po::value<std::string>()->default_value("2.0"), "nrow in grid")
    ("R", po::value<int>()->default_value(100), "nrow in grid")
    ("Q", po::value<int>()->default_value(100), "ncol in grid")
    ("A", po::value<int>()->default_value(10), "number of foragers")
    ("lag", po::value<int>()->default_value(5), "gut retention time")
    ("jspan", po::value<double>()->default_value(10), "lifspan of advanced regeneration")
    ("aspan", po::value<double>()->default_value(100), "lifespan of adult trees")
    ("steps", po::value<int>()->default_value(100), "steps in a day")
    ("years", po::value<int>()->default_value(1000), "years in simulation")
	("init", po::value<std::string>()->default_value("random"), "starting point for landscape and agents")
	("p", po::value<double>()->default_value(0.2), "initial proportion of focal trees")
	("tol", po::value<double>()->default_value(0.12), "")
	("a", po::value<double>()->default_value(0.5), "")
	("E", po::value<double>()->default_value(20), "")
	("f", po::value<double>()->default_value(1.0), "proportion of trees that fruit each day")
;

po::variables_map params;
po::store(po::parse_command_line(argc, argv, desc), params);
po::notify(params);
push_params(params);

std::string init = params["init"].as<std::string>();
double p = params["p"].as<double>();

parameters par(
	params["A"].as<int>(),
	params["lag"].as<int>(),
	params["jspan"].as<double>(),
	params["aspan"].as<double>(),
	params["steps"].as<int>(),
	params["years"].as<int>(),
	params["tol"].as<double>(),
	params["a"].as<double>(),
	params["E"].as<double>(),
	params["f"].as<double>()
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
std::ofstream amfile("annual");

//SIMULATION
for(int day = 0; day != par.days; ++day) {
	shuffle(foragers.begin(), foragers.end(), urng());
	for(int step = 0; step != par.steps; ++step) {
		// TIME STEP
		for(auto& A : foragers) BiModalStep(A, forest, hex, par, state, day);
	}
	//END OF THE DAY
	for(auto& A: foragers) {seedflush(A, forest); memflush(A);}
	state.update();
	// DAILY METRICS
	/**/
	phen(forest, hex, par, state, day);
	grow(forest, ngaps, par, state, day);
	// YEARLY METRICS 
	if(day % 365 == 0) amfile<< day <<" "<< state.Ed <<" "<< abundance(hex, state) <<endl;
}

// ENDPOINT METRICS
/**/
// STATE DUMP
push_state(forest, foragers);

// Close files
//dmfile.close();
amfile.close();

return 0;
};

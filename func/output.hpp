#include <fstream>
#include <iostream>

template <typename land>
void push_land(const land& L, int t, const std::string& run_id, std::function<bool (int)> in_range) {
  if(in_range(t)) {
	// Resource
	std::ofstream map;
	map.open ("res/" + run_id + "m", std::ios::app);
	for(auto C = L.begin(); C != L.end() - 1; ++C) map << C->resource;
	map << L.back().resource << std::endl;
	map.close();
	// True Bank
	std::ofstream Tbank;
	Tbank.open("res/" + run_id + "tb", std::ios::app);
	for(auto C = L.begin(); C != L.end() - 1; ++C) Tbank << C->bank[1] << ",";
	Tbank << L.back().bank[1] << std::endl;
	Tbank.close();
  }
}

template <typename pop>
void push_pop(const pop& P, int t, const std::string& run_id, std::function<bool (int)> in_range) {
  if(in_range(t)) {
	// Agent Locations
	std::ofstream aloc;
	aloc.open("res/" + run_id + "a", std::ios::app);
	for(auto A = P.begin(); A != P.end() - 1; ++A) aloc << A->loc << ",";
	aloc << P.back().loc << std::endl;
	aloc.close();
  }
}

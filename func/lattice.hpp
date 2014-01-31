#include <vector> 

std::vector<int> grid_adj(int pos, int m, int n) {
	int row = pos / n; int col = pos % n;
	std::vector<int> adj;
	// right
		
	// down

	// left

	// up
	return adj;
}



std::vector<int> mesh_adj(int pos, int m, int n) { // convention: odd rows shift right
	int row = pos / n; int col = pos % n; bool odd = row % 2;
	std::vector<int> adj;
	// 1 o'clock
	if(row < m-1 && (!odd || col < n-1)) adj.push_back(pos + n + odd);
	// 3 o'clock
	if(col < n-1) adj.push_back(pos + 1);
	// 5 o'clock
	if(row > 0 && (!odd || col < n-1)) adj.push_back(pos - n + odd);
	// 7 o'clock
	if(row > 0 && (odd || col > 0)) adj.push_back(pos - n - 1 + odd);
	// 9 o'clock
	if(col > 0) adj.push_back(pos - 1);
	// 11 o'clock
	if(row < m-1 && (odd || col > 0)) adj.push_back(pos + n - 1 + odd);
	return adj;
}

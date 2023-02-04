#include <fstream>
#include <deque>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>

#include "bufia_algorithm_utils.h"
#include "bufia_init_utils.h"
#include "factor.h"

using ::std::string;
using ::std::vector;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::pair;
using ::std::list;
using ::std::set;

int main(int argc, char **argv) {
	int MAX_FACTOR_WIDTH = 2;
	int MAX_FEATURES_PER_BUNDLE = 3;

	// load files from input flags
	if(argc < 2) {
		std::cout << "Expected at least two arguments: feature file and data file";
		return 1;
	}

	string feature_filename = argv[1];
	string data_filename = argv[2];
	std::ifstream feature_file (feature_filename);
	std::ifstream data_file (data_filename);

	if(!feature_file.is_open()) {
		std::cout << "Failed to open feature file";
		return 1;
	}
	if(!data_file.is_open()) {
		std::cout << "Failed to open data file";
		return 1;
	}

	vector<string> feature_order;
	// symbol -> width-1 Factor
	unordered_map<string, Factor> alphabet = LoadAlphabetFeatures(&feature_file, feature_order);

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet);

	list<Factor> queue = {Factor(vector<vector<char>>(1, 
		vector<char>(feature_order.size(), '*')))};
	vector<Factor> constraints;
	set<Factor> visited;

	while(!queue.empty()) {
		Factor current = queue.front();
		queue.pop_front();
		if(Contains(constraints, current)) continue;
		//visited.insert(current);

		std::cout << "current: "<< current.toString() << std::endl;

		if(Contains(positive_data[current.bundles.size()], current)) {
			std::cout << "found" << std::endl;
			list<Factor> next_factors = current.getNextFactors(alphabet, 
				MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);
			next_factors.remove_if([visited, constraints, current](Factor fac){
				return //visited.find(fac) != visited.end() || 
				Contains(constraints, fac);
			});

			std::cout<< "adding " << next_factors.size() << " new factors" << std::endl;
			queue.splice(queue.end(), next_factors);
			//std::cout << "new queue size: " << queue.size() << std::endl;
		} else {
			std::cout<< "not found, adding constraint" << std::endl;
			constraints.push_back(current);
		}
	}

	std::cout << constraints.size() << ": size" << std::endl;
	for(Factor const& constraint : constraints){
		std::cout << constraint.toString() << std::endl;
	}

	// Factor fac(vector<vector<char>>(
	// 	{{'*', '*', '*', '+', '*', '*', '*', '*'}, {'*', '*', '*', '+', '*', '*', '*', '*'}}));
	// std::cout << positive_data[1].size() << std::endl;
	// std::cout << Contains(positive_data[2], fac) << std::endl;

	return 0;
}

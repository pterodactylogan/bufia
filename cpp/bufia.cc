#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>
#include "factor.h"
#include "bufia_init_utils.h"

using ::std::string;
using ::std::vector;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::pair;
using ::std::list;

int main(int argc, char **argv) {
	int MAX_FACTOR_WIDTH = 2;
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

	unordered_map<string, Factor> alphabet = LoadAlphabetFeatures(&feature_file);

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH);

	std::cout << alphabet.size();
	std::cout << "\n";
	std::cout << alphabet["i"].bundles.at(0).size();
	std::cout << "\n";
	std::cout << alphabet["b"].bundles.at(0).at(2);
	std::cout << "\n";

	return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>
#include "factor.h"
//#include "bufia_init_utils.h"

using ::std::string;
using ::std::vector;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::pair;
using ::std::list;

int main(int argc, char **argv) {
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

	//unordered_map<string, Factor> map = LoadAlphabetFeatures(&feature_file);
	//std::cout << map.size() << std::endl;

	// Look at first line of feature file
	string symbols;
	std::getline(feature_file, symbols);

	size_t pos = symbols.find(",");
	string symbol;
	vector<string> symbol_order;
	// push each new symbol into symbol_order vector
	// TODO: refactor into function
	while (pos != string::npos) {
		symbol = symbols.substr(0, pos);
		
		// handle empty symbols
		if(symbol.empty()){
			if(symbol_order.size() > 0) {
				std::cout << "WARNING: you have a blank entry in" 
				" your symbol list. This may cause unexpected behavior.";
			} else {
				symbols.erase(0, pos+1);
				pos = symbols.find(",");
				continue;
			}
		}

		symbol_order.push_back(symbol);

		symbols.erase(0, pos+1);
		pos = symbols.find(",");
	}
	symbol_order.push_back(symbols);

	// find number of features
	int num_features = -1;
	std::ifstream feature_file_cp (feature_filename);
	if(!feature_file_cp.is_open()) {
		std::cout << "failed to open feature file";
		return 1;
	}
	std::string ln;
	while(std::getline(feature_file_cp, ln)) {
		++num_features;
	}

	// go through rest of lines, fill in map of symbols to Factors
	unordered_map<string, Factor> alphabet;
	string values;
	int feature_i = 0;
	while (std::getline(feature_file, values)) {
		size_t pos = values.find(",");
		string feature_name = values.substr(0, pos);
		values.erase(0, pos+1);

		pos = values.find(",");
		int i = 0;
		string value;
		while (i<symbol_order.size()) {
			value = values.substr(0, pos);
			if(value != "0" && value!="+" && value !="-"){
				std::cout << "Invalid value symbol: " + value + ". This may cause"
				" unexpected behavior";
			}

			if (alphabet.find(symbol_order[i]) == alphabet.end()) {
				alphabet[symbol_order[i]] = Factor(1, num_features);
			}
			alphabet[symbol_order[i]].bundles.at(0)[feature_i] = value[0];

			values.erase(0, pos+1);
			pos = values.find(",");
			++i;
		}
		++feature_i;
	}

	std::cout << alphabet.size();
	std::cout << "\n";
	std::cout << alphabet["i"].bundles.at(0).size();
	std::cout << "\n";
	std::cout << alphabet["b"].bundles.at(0).at(2);
	std::cout << "\n";

	return 0;
}

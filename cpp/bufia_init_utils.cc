#include "bufia_init_utils.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include "factor.h"

using ::std::string;
using ::std::unordered_map;

unordered_map<string, Factor> 
LoadAlphabetFeatures(std::ifstream* feature_file){

	// find number of features
	int num_features = -1;
	std::string ln;
	while(std::getline(*feature_file, ln)) {
		++num_features;
	}

	feature_file->clear();
	feature_file->seekg(0);

	// Look at first line of feature file
	string symbols;
	std::getline(*feature_file, symbols);

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

	// go through rest of lines, fill in map of symbols to Factors
	unordered_map<string, Factor> alphabet;
	string values;
	int feature_i = 0;
	while (std::getline(*feature_file, values)) {
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
	return alphabet;
}

unordered_map<int, vector<Factor>> 
LoadPositiveData(std::ifstream* data_file, int max_width){
	unordered_map<int, vector<Factor>> map;
	return map;
}
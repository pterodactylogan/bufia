#include "bufia_init_utils.h"

#include <iostream>
#include <fstream>
#include <set>
#include <unordered_map>
#include <bits/stdc++.h>
#include "factor.h"

using ::std::string;
using ::std::set;
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

// Creates a factor for the ngram represented by `symbols` vector
// If `begin_index` is set, only that index onward will be used in the ngram
Factor MakeFactor(const vector<string>& symbols,
	const unordered_map<string, Factor>& alphabet, int begin_index=0) {
	vector<vector<char>> bundles;
	for(int i=begin_index; i<symbols.size(); i++) {
		if(symbols.at(i)=="#"){
			// Vector of same length populated only with '#' for edge marker
			bundles.push_back(vector<char>(alphabet.begin()->second.bundles.at(0).size(), '#'));
		} else {
			bundles.push_back(vector<char>(alphabet.at(symbols.at(i)).bundles[0]));
		}
	}
	Factor ret(bundles);
	return ret;
}

unordered_map<int, set<Factor>> 
LoadPositiveData(std::ifstream* data_file, int max_width, 
	const unordered_map<string, Factor>& alphabet){

	unordered_map<int, set<Factor>> data;

	string word;
	vector<string> prev;
	while(std::getline(*data_file, word)){
		word = "# " + word + " #";
		std::stringstream word_stream(word);
		string symbol;
		while (std::getline(word_stream, symbol, ' ')){
			prev.push_back(symbol);
			for(int width = 1; width<=max_width; width++){
				if(prev.size() >= width) {
					data[width].insert(MakeFactor(prev, alphabet, prev.size()-width));
				}
			}
		}
		prev.clear();
	}

	return data;
}
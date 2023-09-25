#include "bufia_init_utils.h"

#include <iostream>
#include <fstream>
#include <set>
#include <unordered_map>
#include <sstream>
#include <vector>

#include "factor.h"

using ::std::string;
using ::std::set;
using ::std::unordered_map;
using ::std::vector;

unordered_map<string, Factor> 
LoadAlphabetFeatures(std::ifstream* feature_file, 
	vector<string>& feature_order, string delim){

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

	size_t pos = symbols.find(delim);
	string symbol;
	vector<string> symbol_order;

	// push each new symbol into symbol_order vector
	while (pos != string::npos) {
		symbol = symbols.substr(0, pos);
		
		// handle empty symbols
		if(symbol.empty()){
			if(symbol_order.size() > 0) {
				std::cout << "WARNING: you have a blank entry in" << 
				" your symbol list. This may cause unexpected behavior." << std::endl;
			} else {
				symbols.erase(0, pos+1);
				pos = symbols.find(delim);
				continue;
			}
		}

		symbol_order.push_back(symbol);

		symbols.erase(0, pos+1);
		pos = symbols.find(delim);
	}
	symbol_order.push_back(symbols);

	// go through rest of lines, fill in map of symbols to Factors
	unordered_map<string, Factor> alphabet;
	string values;
	int feature_i = 0;
	while (std::getline(*feature_file, values)) {
		size_t pos = values.find(delim);
		string feature_name = values.substr(0, pos);
		feature_order.push_back(feature_name);
		values.erase(0, pos+1);

		pos = values.find(delim);
		int i = 0;
		string value;
		while (i<symbol_order.size()) {
			value = values.substr(0, pos);
			if(value != "0" && value!="+" && value !="-"){
				std::cout << "Invalid value symbol: " << value << ". This may cause"
				" unexpected behavior" << std::endl;
			}

			if (alphabet.find(symbol_order[i]) == alphabet.end()) {
				alphabet[symbol_order[i]] = Factor(1, num_features);
			}
			alphabet[symbol_order[i]].bundles.at(0)[feature_i] = value[0];

			values.erase(0, pos+1);
			pos = values.find(delim);
			++i;
		}
		++feature_i;
	}
	return alphabet;
}

// Creates a factor for the ngram represented by `symbols` vector
// If `begin_index` is set, only that index onward will be used in the ngram
// If `end_index` is set, only up to that index will be used in the ngram
Factor MakeFactor(const vector<string>& symbols,
	const unordered_map<string, Factor>& alphabet, int begin_index=0,
	int end_index=-1) {
	vector<vector<char>> bundles;
	if(end_index < 0 || end_index > symbols.size()) end_index = symbols.size();
	if(begin_index <0) begin_index = 0;

	for(int i=begin_index; i<end_index; i++) {
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

set<Factor> GetSubsequences(const vector<string>& word, int max_width,
	const unordered_map<string, Factor>& alphabet) {
	set<Factor> result;
	if(max_width == 0) {
		result.insert(Factor());
		return result;
	}

	// for each symbol in word
	for(int i=0; i<=word.size(); ++i) {
		// make it a factor
		Factor symb = MakeFactor(word, alphabet, i, i+1);
		set<Factor> following_seq = GetSubsequences(vector<string>(word.begin()+1, 
			word.end()), max_width-1, alphabet);
		// concat with everything in following subsequences and add to result
		for(const auto& seq : following_seq) {
			Factor combo = symb;
			combo.append(seq);
			result.insert(combo);
		}
	}
	return result;
}

unordered_map<int, vector<Factor>> 
LoadPositiveData(std::ifstream* data_file, int max_width, 
	const unordered_map<string, Factor>& alphabet, int order){

	unordered_map<int, set<Factor>> data;

	if(order == 1) {
		// Successor
		string word;
		vector<string> prev;
		while(std::getline(*data_file, word)){
			word = "# " + word + " #";
			std::stringstream word_stream;
			word_stream << word;
			string symbol;
			while (std::getline(word_stream, symbol, ' ')){
				if(symbol == "") {
					std::cout << "Your training data may not be properly formatted."
						<< " Check for trailing spaces. Data item: " << word << std::endl;
					continue;
				}
				prev.push_back(symbol);
				for(int width = 1; width<=max_width; width++){
					if(prev.size() >= width) {
						data[width].insert(MakeFactor(prev, alphabet, prev.size()-width));
					}
				}
			}
			prev.clear();
		}
	} else {
		// Precedence
		string word;
		while(std::getline(*data_file, word)) {
			word = "# " + word + " #";
			std::stringstream word_stream;
			word_stream << word;
			string symbol;
			vector<string> symbols;
			while(std::getline(word_stream, symbol, ' ')){
				if(symbol == "") {
					std::cout << "Your training data may not be properly formatted."
						<< " Check for trailing spaces. Data item: " << word << std::endl;
					continue;
				}
				symbols.push_back(symbol);
			}
			set<Factor> subseqs = GetSubsequences(symbols, max_width, alphabet);
			for(const auto& seq : subseqs) {
				data[seq.size()].insert(seq);
			}
		}
	}

	unordered_map<int, vector<Factor>> result;
	for(const auto& pair : data){
		result[pair.first] = vector<Factor>(pair.second.begin(), pair.second.end());
	}	
	return result;
}

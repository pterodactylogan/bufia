#include "bufia_init_utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "factor.h"

using ::std::multiset;
using ::std::pair;
using ::std::string;
using ::std::set;
using ::std::unordered_map;
using ::std::vector;

unordered_map<string, Factor> 
LoadAlphabetFeatures(std::ifstream* feature_file, 
	vector<string>& feature_order, vector<pair<int, char>>& feature_ranks,
	string delim, bool add_wb, int rank_features, vector<string> tier){

	bool has_wb = false;

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
		if(symbol == "#") has_wb = true;
		
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
	if(!has_wb && add_wb) ++num_features;
	symbol_order.push_back(symbols);

	// go through rest of lines, fill in map of symbols to Factors
	unordered_map<string, Factor> alphabet;
	string values;
	int feature_i = 0;
	feature_order = vector<string>();
	std::set<pair<int, pair<int, char>>> feature_set;
	while (std::getline(*feature_file, values)) {
		size_t pos = values.find(delim);
		string feature_name = values.substr(0, pos);
		feature_order.push_back(feature_name);
		values.erase(0, pos+1);

		pos = values.find(delim);
		int i = 0;
		int pos_count = 0;
		int neg_count = 0;
		string value;
		while (i<symbol_order.size()) {
			// add symbol to alphabet only if it is on the tier
			if(tier.empty() || 
				std::find(tier.begin(), tier.end(), symbol_order[i]) != tier.end()) {
				value = values.substr(0, pos);
				if(value != "0" && value!="+" && value !="-"){
					std::cout << "Invalid value symbol: " << value << ". This may cause"
					" unexpected behavior" << std::endl;
				}

				if (alphabet.find(symbol_order[i]) == alphabet.end()) {
					alphabet[symbol_order[i]] = Factor(1, num_features);
				}
				alphabet[symbol_order[i]].bundles.at(0)[feature_i] = value[0];

				if(value[0] == '+') ++pos_count;
				if(value[0] == '-') ++neg_count;
			}
 
			values.erase(0, pos+1);
			pos = values.find(delim);
			++i;
		}
		feature_set.insert({pos_count, {feature_i, '+'}});
		feature_set.insert({neg_count, {feature_i, '-'}});
		++feature_i;
	}
	if(!has_wb && add_wb) {
		// fill in -wb for all symbols
		for (const auto& entry : alphabet) {
			alphabet[entry.first].bundles.at(0)[feature_i] = '-';
		}

		// add wb to feature order
		feature_order.push_back("wb");

		// add "#" to alphabet
		vector<char> wb(num_features, '0');
		wb[feature_i] = '+';
		alphabet["#"] = Factor({wb});

		feature_set.insert({1, {feature_i, '+'}});
		feature_set.insert({symbol_order.size(), {feature_i, '-'}});
	}

	if(rank_features) {
		feature_ranks = vector<pair<int, char>>();
		for(const auto& count : feature_set) {
			feature_ranks.insert(feature_ranks.begin(),
				std::make_pair(count.second.first, count.second.second));
		}
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
		if (alphabet.find(symbols.at(i)) == alphabet.end()) {
			std::cout << "Symbol " << symbols.at(i) << " is not present in feature file." << std::endl;
			continue;
		}
		bundles.push_back(vector<char>(alphabet.at(symbols.at(i)).bundles[0]));
	}
	Factor ret(bundles);
	return ret;
}

set<Factor> GetSubsequences(const vector<string>& word, int max_width,
	const unordered_map<string, Factor>& alphabet) {
	set<Factor> result;
	if(max_width == 0 || word.size() == 0) {
		result.insert(Factor());
		return result;
	}

	// for each symbol in word
	for(int i=0; i<=word.size(); ++i) {
		// make it a factor
		Factor symb = MakeFactor(word, alphabet, i, i+1);

		set<Factor> following_seq({Factor()});
		if(i<word.size()) {
			following_seq = GetSubsequences(vector<string>(word.begin()+1+i, 
				word.end()), max_width-1, alphabet);
		}
		// concat with everything in following subsequences and add to result
		for(const auto& seq : following_seq) {
			Factor combo = symb;
			combo.append(seq);
			result.insert(combo);
		}
	}
	return result;
}

unordered_map<int, vector<pair<Factor, int>>> 
LoadPositiveData(std::ifstream* data_file, int max_width, 
	const unordered_map<string, Factor>& alphabet, 
	const vector<string>& tier, int order,
	bool add_wb){

	unordered_map<int, multiset<Factor>> data;

	if(order == 1 || !tier.empty()) {
		// Successor or tier projection
		string word;
		vector<string> prev;
		// go through all words in file
		while(std::getline(*data_file, word)){
			if(add_wb) word = "# " + word + " #";

			std::stringstream word_stream;
			word_stream << word;
			string symbol;
			while (std::getline(word_stream, symbol, ' ')){
				if(symbol == "") {
					std::cout << "Your training data may not be properly formatted."
						<< " Check for trailing spaces. Data item: " << word << std::endl;
					continue;
				}
				// project the symbol if it is in the tier (or if no tier was given)
				if(tier.empty() || std::find(tier.begin(), tier.end(), symbol)
					!= tier.end()) {
					prev.push_back(symbol);
				}

				for(int width = 1; width<=max_width; width++){
					if(prev.size() >= width) {
						data[width].insert(MakeFactor(prev, alphabet, 
							/*begin_index=*/prev.size()-width));
					}
				}
			}
			prev.clear();
		}
	} else {
		// Precedence
		string word;
		while(std::getline(*data_file, word)) {
			if(add_wb) word = "# " + word + " #";
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

	unordered_map<int, vector<pair<Factor, int>>> result;
	for(const auto& symbol : data){
		vector<pair<Factor, int>> facs;
		auto it = symbol.second.begin();
		while(it != symbol.second.end()){
			int count = symbol.second.count(*it);
			facs.push_back({*it, count});
			it = symbol.second.upper_bound(*it);
		}
		result[symbol.first] = facs;
	}	
	return result;
}

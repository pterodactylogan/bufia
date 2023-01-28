#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>

using ::std::string;
using ::std::vector;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::pair;
using ::std::list;

int main(int argc, char **argv) {
	// load feature file from flag
	string feature_filename = argv[1];
	std::ifstream feature_file (feature_filename);

	if(!feature_file.is_open()) {
		std::cout << "Failed to open feature file";
		return 1;
	}

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

	// go through rest of lines, fill in vector of vectors
	// populate map feature_name -> value -> characters
	vector<std::unique_ptr<list<char>>> feature_bundles(symbol_order.size());
	unordered_map<string, 
		pair<unordered_set<string>, unordered_set<string>>> feature_to_symbols;
	string values;
	while (std::getline(feature_file, values)) {
		size_t pos = values.find(",");
		string feature_name = values.substr(0, pos);
		values.erase(0, pos+1);

		unordered_set<string> plus;
		unordered_set<string> minus;
		feature_to_symbols[feature_name] = std::make_pair(plus, minus);

		pos = values.find(",");
		int i = 0;
		string value;
		while (i<symbol_order.size()) {
			value = values.substr(0, pos);
			if(value == "+"){
				feature_to_symbols[feature_name].first.insert(symbol_order.at(i));
			} else if(value == "-"){
				feature_to_symbols[feature_name].second.insert(symbol_order.at(i));
			} else if(value != "0"){
				std::cout << "Invalid value symbol: " + value + ". This may cause"
				" unexpected behavior";
			}

			if(feature_bundles.at(i) == nullptr){
				feature_bundles[i] = std::unique_ptr<list<char>>(new list<char>());
			}
			feature_bundles.at(i)->push_back(value[0]);

			values.erase(0, pos+1);
			pos = values.find(",");
			++i;
		}
	}

	std::cout << feature_to_symbols["cons"].first.size();
	std::cout << "\n";
	std::cout << feature_to_symbols["cons"].second.size();
	std::cout << "\n";

	for(int i=0; i<feature_bundles.size(); i++){
		for(const auto& item : *feature_bundles.at(i)) {
			std::cout << item;
			std::cout << ",";
		}
		std::cout << "\n";
	}

	// convert vector of vectors to map of char -> factor

	return 0;
}

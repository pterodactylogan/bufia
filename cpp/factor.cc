#include "factor.h"

#include <iostream>

#include <list>
#include <vector>
#include <unordered_map>

using ::std::list;
using ::std::unordered_map;
using ::std::vector;

bool Factor::operator < (const Factor& other) const {
	if(other.bundles.size() != bundles.size()) return false;
	for(int i=0; i<bundles.size(); i++) {
		for(int j=0; j<bundles.at(i).size(); j++) {
			if(other.bundles.at(i).size() != bundles.at(i).size()) return false;
			char s = bundles[i][j];
			char o = other.bundles[i][j];

			if(o==s) continue;

			if(o == '#') return false;
			if(s == '#') return true;

			if(o == '*') return false;
			if(s == '*') return true;

			if(o == '0') return false;
			if(s == '0') return true;

			if(o == '-') return false;
			if(s == '-') return true;

			if(o == '+') return false;
			if(s == '+') return true;
		}
	}
	return false;
}

bool Factor::operator > (const Factor& other) const {	
	if(other.bundles.size() != bundles.size()) return false;
	for(int i=0; i<bundles.size(); i++) {
		for(int j=0; j<bundles.at(i).size(); j++) {
			if(other.bundles.at(i).size() != bundles.at(i).size()) return false;
			char s = bundles[i][j];
			char o = other.bundles[i][j];

			if(o==s) continue;

			if(o == '#') return true;
			if(s == '#') return false;

			if(o == '*') return true;
			if(s == '*') return false;

			if(o == '0') return true;
			if(s == '0') return false;

			if(o == '-') return true;
			if(s == '-') return false;

			if(o == '+') return true;
			if(s == '+') return false;
		}
	}
	return false;
}

std::string Factor::toString() const {
	std::string result = "";
	for(const auto& bundle : bundles){
		result += "[";
		for(const auto& value : bundle) {
			result+= std::string(1, value) + ", ";
		}
		result +="]\n";
	}
	return result;
}

list<Factor> Factor::getNextFactors(
	const unordered_map<std::string, Factor>& alphabet, 
	int max_width, int max_features) const {
	if(bundles[0][0]=='#') return {};

	list<Factor> result;
	int last = bundles.size()-1;
	int i = bundles[last].size()-1;
	int unset_index = 0;
	int num_features = 0;
	while(i>=0){
		if(bundles[last][i] != '*') {
			++num_features;
			if(unset_index == 0) unset_index = i+1;
		}
		--i;
	}

	std::cout<<num_features<<std::endl;

	if(num_features < max_features) {
		for(unset_index; unset_index<bundles[last].size(); unset_index++){
			vector<vector<char>> next_pos = bundles;
			next_pos[last][unset_index] = '+';
			// check if this generates anything in alphabet
			result.push_back(Factor(next_pos));

			vector<vector<char>> next_neg = bundles;
			next_neg[last][unset_index] = '-';
			// ditto
			result.push_back(Factor(next_neg));
		}
	}

	if(bundles.size() < max_width) {
		vector<vector<char>> next_begin;
		next_begin.push_back(vector<char>(bundles[last].size(), '#'));
		next_begin.insert(next_begin.end(), bundles.begin(), bundles.end());
		result.push_back(Factor(next_begin));

		vector<vector<char>> next_end = bundles;
		next_end.push_back(vector<char>(bundles[last].size(), '#'));
		result.push_back(Factor(next_end));

		vector<vector<char>> next_blank = bundles;
		next_blank.push_back(vector<char>(bundles[last].size(), '*'));
		result.push_back(Factor(next_blank));
	}
	return result;
}

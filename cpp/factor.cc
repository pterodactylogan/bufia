#include "factor.h"

#include <iostream>

#include <list>
#include <vector>
#include <unordered_map>

using ::std::list;
using ::std::unordered_map;
using ::std::vector;

bool Factor::operator < (const Factor& other) const {
	if(other.bundles.size() > bundles.size()) return true;
	if(other.bundles.size() < bundles.size()) return false;
	for(int i=0; i<bundles.size(); i++) {
		for(int j=0; j<bundles.at(i).size(); j++) {
			if(other.bundles.at(i).size() > bundles.at(i).size()) return true;
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
	if(other.bundles.size() < bundles.size()) return true;	
	if(other.bundles.size() > bundles.size()) return false;
	for(int i=0; i<bundles.size(); i++) {
		for(int j=0; j<bundles.at(i).size(); j++) {
			if(other.bundles.at(i).size() < bundles.at(i).size()) return true;
			if(other.bundles.at(i).size() > bundles.at(i).size()) return false;
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

bool Factor::generates(const Factor& child) const {
	if(bundles.size() == 0 || child.bundles.size() == 0) return false;
	if(bundles.size() > child.bundles.size()) return false;
	if(bundles.at(0).size() != child.bundles.at(0).size()) return false;

	// If same size or this starts with #, anchor left
	if(bundles.size() == child.bundles.size() || bundles[0][0] == '#'){
		for(int i=0; i<bundles.size(); i++){
			for(int j=0; j<bundles.at(0).size(); j++){
				if(bundles[i][j] == '*') continue;
				if(bundles[i][j] != child.bundles[i][j]) return false;
			}
		}
		return true;
	}

	// If this ends with #, anchor right
	if(bundles[bundles.size()-1][0] == '#'){
		for(int i=1; i<=bundles.size(); i++){	
			for(int j=0; j<bundles.at(0).size(); j++){
				if(bundles[bundles.size()-i][j] == '*') continue;
				if(bundles[bundles.size()-i][j] != 
					child.bundles[child.bundles.size()-i][j]) return false;
			}
		}
		return true;
	}

	// Otherwise, check all k-size substrings
	for(int offset = 0; offset <= child.bundles.size() - bundles.size(); 
		offset++) {
		bool found_mismatch = false;
		for(int i=0; i<bundles.size(); i++){
			if(found_mismatch) break;
			for(int j=0; j<bundles.at(0).size(); j++){
				if(bundles[i][j] == '*') continue;
				if(bundles[i][j] != child.bundles[i+offset][j]){
					found_mismatch = true;
					break;
				}
			}
		}
		if(!found_mismatch) return true;
	}
	return false;
}

list<Factor> Factor::getNextFactors(
	const unordered_map<std::string, Factor>& alphabet, 
	int max_width, int max_features) const {
	if(bundles[0][0]=='#' || bundles[bundles.size()-1][0] == '#') return {};

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

	if(num_features < max_features) {
		for(; unset_index<bundles[last].size(); unset_index++){
			vector<vector<char>> next_pos = bundles;
			next_pos[last][unset_index] = '+';

			// check if last bundle generates anything in alphabet
			Factor final({next_pos[last]});
			for(const auto& pair : alphabet){
				if(final.generates(pair.second)){
					result.push_back(Factor(next_pos));
					break;
				}
			}

			vector<vector<char>> next_neg = bundles;
			next_neg[last][unset_index] = '-';
			final = Factor({next_neg[last]});
			for(const auto& pair : alphabet){
				if(final.generates(pair.second)){
					result.push_back(Factor(next_neg));
					break;
				}
			}
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

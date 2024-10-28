#include "factor.h"

#include <iostream>

#include <list>
#include <set>
#include <vector>
#include <unordered_map>

using ::std::list;
using ::std::set;
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

bool Factor::operator == (const Factor& other) const {
	return (!(*this < other) && !(*this > other));
}

int Factor::size() const {
	return bundles.size();
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

// get all subsequences of a factor of the specified `width`
set<Factor> Factor::getSubsequences(int width) const {

	set<Factor> result;
	if(width == 0 || bundles.size() == 0) {
		result.insert(Factor());
		return result;
	}

	// for each symbol
	for(int i=0; i<bundles.size(); ++i) {
		// chop it off
		Factor seg = Factor({bundles[i]});

		set<Factor> following_seq;
		if(i + width - 1 < bundles.size()) {
			following_seq = Factor(vector<vector<char>>(bundles.begin()+i+1, bundles.end()))
								.getSubsequences(width-1);
		}
		// concat with everything in following subsequences and add to result
		for(const auto& seq : following_seq) {
			Factor combo = seg;
			combo.append(seq);
			result.insert(combo);
		}
	}
	return result;
}

bool Factor::generates(const Factor& child, int order) const {
	if(bundles.size() == 0 || child.bundles.size() == 0) return false;
	if(bundles.size() > child.bundles.size()) return false;
	if(bundles.at(0).size() != child.bundles.at(0).size()) return false;
	
	if(order ==1) {
		// check all k-size substrings
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
	} else {
		for(const auto& seq : child.getSubsequences(bundles.size())) {
			bool found_mismatch = false;
			for (int i=0; i<bundles.size(); i++){
				if(found_mismatch) break;
				for(int j=0; j<bundles.at(0).size(); j++){
					if(bundles[i][j] == '*') continue;
					if(bundles[i][j] != seq.bundles[i][j]){
						found_mismatch = true;
						break;
					}
				}
			}
			if(!found_mismatch) return true;
		}
	}
	return false;
}

list<Factor> Factor::getNextFactors(
	const unordered_map<std::string, Factor>& alphabet, 
	int max_width, int max_features, int max_dist,
	vector<std::pair<int, char>>* feature_ranks) const {

	list<Factor> result;

	if(max_dist > -1){
		int dist = 0;
		for(int i = 0; i<bundles.size(); ++i){
			++dist;

			for(int j=0; j<bundles[i].size(); ++j){
				if(bundles[i][j] != '*') ++dist;
			}
		}

		if(dist >= max_dist) {
			return result;
		}
	}

	int last = bundles.size()-1;

	// find latest unset index and number of set features
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

	if(feature_ranks ==nullptr) {
		if(max_features == -1 || num_features < max_features) {
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
	} else {
		if(max_features == -1 || num_features < max_features) {
			for(int i = feature_ranks->size()-1; i>=0; --i) {

				// pair of index, value to represent an element like -son
				std::pair<int, char> feat_pair = feature_ranks->at(i);

				// if index is occupied by the relevant value, break
				if(bundles[last][feat_pair.first] == feat_pair.second) {
					break;
				}

				// otherwise add elem (if opposite elem is not present)
				if(bundles[last][feature_ranks->at(i).first] == '*') {
					vector<vector<char>> next = bundles;
					next[last][feature_ranks->at(i).first] = feat_pair.second;

					// check if last bundle generates anything in alphabet
					Factor final({next[last]});
					for(const auto& pair : alphabet){
						if(final.generates(pair.second)){
							result.push_front(Factor(next));
							break;
						}
					}
				}
			}
		}
	}

	if(max_width == -1 || bundles.size() < max_width) {
		vector<vector<char>> next_blank = bundles;
		next_blank.push_back(vector<char>(bundles[last].size(), '*'));
		result.push_back(Factor(next_blank));
	}
	
	return result;
}



void Factor::append(const Factor& other) {
	bundles.insert(bundles.end(), other.bundles.begin(), other.bundles.end());
}

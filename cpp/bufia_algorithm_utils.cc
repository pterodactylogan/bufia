#include "bufia_algorithm_utils.h"

#include <omp.h>
#include <iostream>
#include <set>
#include <unordered_map>

#include "factor.h"

using ::std::vector;
using ::std::string;
using ::std::set;

bool Contains(const vector<Factor>& positive_data,
	const Factor& parent) {
	bool found = false;
	#pragma omp parallel shared(found) 
	{
		#pragma omp for
		for(int i=0; i<positive_data.size(); i++){
			if(parent.generates(positive_data[i])){
				#pragma omp critical
				{
					found = true;
				}
				#pragma omp cancel for
			}
		}
	}
	return found;
}

bool Covers(const vector<Factor>& constraints, const Factor& child, int order) {
	bool found = false;
	#pragma omp parallel shared(found)
	{
		#pragma omp for
		for(int i=0; i<constraints.size(); i++){
			if(constraints[i].generates(child, order)){
				#pragma omp critical
				{
					found = true;
				}
				#pragma omp cancel for
			}
		}
	}
	return found;
}

string Display(const Factor& fac, const vector<string>& feature_order) {
	string result;
	for(const auto& bundle : fac.bundles){
		if(bundle.size() > 0 && bundle[0] == '#') {
			result += "#";
			continue;
		}
		result += "[";
		for(int i=0; i<bundle.size(); i++) {
			if(bundle.at(i) == '+' || bundle.at(i) == '-') {
				result += bundle.at(i) + feature_order.at(i) + ",";
			}
		}
		if(result[result.size()-1] == ',') result.pop_back();
		result += "]";
	}
	return result;
}

vector<vector<string>> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet) {
	if(fac.bundles.size() == 0) return {{}};

	vector<vector<string>> tails = ComputeGeneratedNGrams(
		Factor(vector<vector<char>>(fac.bundles.begin()+1, 
			fac.bundles.end())), alphabet);
	vector<vector<string>> result;
	Factor first({fac.bundles.at(0)});

	if(first.bundles.at(0).at(0) == '#'){
		for(const auto& tail : tails) {
			vector<string> concat = {"#"};
			concat.insert(concat.end(), tail.begin(), tail.end());
			result.push_back(concat);
		}
		return result;
	}

	for(const auto& pair : alphabet) {
		if(first.generates(pair.second)){
			for(const auto& tail : tails){
			vector<string> concat = {pair.first};
			concat.insert(concat.end(), tail.begin(), tail.end());
				result.push_back(concat);
			}
		}
	}
	return result;
}

set<vector<string>> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet,
	int size) {
	if(fac.bundles.size() == 0 || fac.bundles.size() > size) return {{}};

	set<vector<string>> result;
	int diff = size - fac.bundles.size();
	for(int i=0; i<=diff; ++i){
		vector<char> blank(fac.bundles[0].size(), '*');
		Factor padded = Factor(vector<vector<char>>(i, blank));
		padded.append(fac);
		padded.append(Factor(vector<vector<char>>(diff-i, blank)));
		for(const auto& ngram : ComputeGeneratedNGrams(padded, alphabet)){
			result.insert(ngram);
		}
	}
	return result;
}

#include "bufia_algorithm_utils.h"

#include <omp.h>
#include <iostream>
#include <set>
#include <unordered_map>

#include "factor.h"

using ::std::vector;
using ::std::string;

bool Contains(const vector<Factor>& positive_data,
	const Factor& parent) {
	bool found = false;
	#pragma omp parallel for shared(found)
		for(int i=0; i<positive_data.size(); i++){
			// TODO: is there a way to actually make all threads stop?
			if(found) continue;

			if(parent.generates(positive_data[i])){
				found = true;
			}
		}
	return found;
}

bool Covers(const vector<Factor>& constraints, const Factor& child) {
	bool found = false;
	#pragma omp parallel for shared(found)
		for(int i=0; i<constraints.size(); i++){
			//TODO: ditto above
			if(found) continue;

			if(constraints[i].generates(child)){
				found = true;
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

vector<string> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet) {
	if(fac.bundles.size() == 0) return {""};

	vector<string> tails = ComputeGeneratedNGrams(
		Factor(vector<vector<char>>(fac.bundles.begin()+1, 
			fac.bundles.end())), alphabet);
	vector<string> result;
	Factor first({fac.bundles.at(0)});

	if(first.bundles.at(0).at(0) == '#'){
		for(const auto& tail : tails) {
			result.push_back("#" + tail);
		}
		return result;
	}

	for(const auto& pair : alphabet) {
		if(first.generates(pair.second)){
			for(const auto& tail : tails){
				result.push_back(pair.first + tail);
			}
		}
	}
	return result;
}

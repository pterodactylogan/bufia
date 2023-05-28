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
	#pragma omp parallel default(none) shared(found, positive_data, parent)
	{
		#pragma omp single
		{
			for(int i=0; i<positive_data.size(); i++){
				if(found) {
					printf("breaking\n");
					break;
				}
				Factor fac = positive_data[i];
				#pragma omp task firstprivate(parent, fac) shared(found)
				{
					printf("%d\n", omp_get_thread_num());
					if(parent.generates(fac)){
						#pragma omp critical
						{
							found = true;
						}
							printf("found\n");
					}
				}
			}
		}
		#pragma omp taskwait
	}
	std::cout << "returning" << std::endl;
	return found;
}

bool Covers(const vector<Factor>& constraints, const Factor& child) {
	bool found = false;
	#pragma omp parallel default(none) shared(constraints, found, child)
	{
		#pragma omp single
		{
			for(int i=0; i<constraints.size(); i++){
				if(found) break;
				Factor fac = constraints[i];
				#pragma omp task firstprivate(child, fac) shared(found)
				{
					if(fac.generates(child)){
						#pragma omp critical
							found = true;
					}
				}
			}
		}
		#pragma omp taskwait
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

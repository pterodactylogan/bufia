#ifndef BUFIA_ALGORITHM_UTILS_H
#define BUFIA_ALGORITHM_UTILS_H

#include <set>
#include <unordered_map>

#include "factor.h"

using ::std::vector;
using ::std::pair;

//TODO: possibly rename these

// Returns true if any Factor in `positive_data` can be generated from `parent`
// false otherwise.
bool Contains(const vector<pair<Factor, int>>& positive_data,
	const Factor& parent, int ignore_count = 0);

// Returns true if any Factor in `constraints` generates `child`, false
// otherwise.
bool Covers(const vector<Factor>& constraints, const Factor& child, int order = 1);

std::string Display(const Factor& fac, 
	const vector<std::string>& feature_order);

vector<vector<std::string>> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet);

std::set<vector<std::string>> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet,
	int size);

#endif /* BUFIA_ALGORITHM_UTILS_H */

#ifndef BUFIA_ALGORITHM_UTILS_H
#define BUFIA_ALGORITHM_UTILS_H

#include <set>
#include <unordered_map>

#include "factor.h"

using ::std::vector;

//TODO: possibly rename these

// Returns true if any Factor in `positive_data` can be generated from `parent`
// false otherwise.
bool Contains(const vector<Factor>& positive_data,
	const Factor& parent);

// Returns true if any Factor in `constraints` generates `child`, false
// otherwise.
bool Covers(const vector<Factor>& constraints, const Factor& child);

std::string Display(const Factor& fac, 
	const vector<std::string>& feature_order);

vector<std::string> ComputeGeneratedNGrams(const Factor& fac,
	const std::unordered_map<std::string, Factor>& alphabet);

#endif /* BUFIA_ALGORITHM_UTILS_H */

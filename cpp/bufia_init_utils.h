#ifndef BUFIA_INIT_UTILS_H
#define BUFIA_INIT_UTILS_H

#include <fstream>
#include <unordered_map>
#include <set>
#include "factor.h"

// Takes a file object with the features for each character in the alphabet.
// Returns a map from alphabet entries to corresponding 1-dimensional factors.
std::unordered_map<std::string, Factor> 
	LoadAlphabetFeatures(std::ifstream* feature_file);

// Takes a file object with positive training data, an alphabet mapping symbols
// to corresponding factors, and max factor width to consider.
// Returns a map from factor width to the set of Factors with that width present
// in the data.
std::unordered_map<int, std::set<Factor>> 
	LoadPositiveData(std::ifstream* data_file, int max_width,
	const std::unordered_map<std::string, Factor>& alphabet);

#endif /* BUFIA_INIT_UTILS_H */

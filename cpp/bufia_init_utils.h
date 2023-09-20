#ifndef BUFIA_INIT_UTILS_H
#define BUFIA_INIT_UTILS_H

#include <fstream>
#include <set>
#include <unordered_map>
#include <vector>

#include "factor.h"

// Takes a file object with the features for each character in the alphabet.
// Returns a map from alphabet entries to corresponding 1-dimensional factors.
// `feature_order` will be populated with the feature names in the order
// they appear in the file.
std::unordered_map<std::string, Factor> 
	LoadAlphabetFeatures(std::ifstream* feature_file, 
		std::vector<std::string>& feature_order);

// Takes a file object with positive training data, an alphabet mapping symbols
// to corresponding factors, and max factor width to consider.
// Returns a map from factor width to the set of Factors with that width present
// in the data.
std::unordered_map<int, std::vector<Factor>> 
	LoadPositiveData(std::ifstream* data_file, int max_width,
	const std::unordered_map<std::string, Factor>& alphabet,
	int order = 1);

#endif /* BUFIA_INIT_UTILS_H */

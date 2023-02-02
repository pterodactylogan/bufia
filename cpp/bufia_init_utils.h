#ifndef BUFIA_INIT_UTILS_H
#define BUFIA_INIT_UTILS_H

#include <fstream>
#include <unordered_map>
#include "factor.h"

// Takes a file object with the features for each character in the alphabet.
// Returns a map from alphabet entries to corresponding 1-dimensional factors.
std::unordered_map<std::string, Factor> 
	LoadAlphabetFeatures(std::ifstream* feature_file);

std::unordered_map<int, std::vector<Factor>> 
	LoadPositiveData(std::ifstream* data_file, int max_width);

#endif /* BUFIA_INIT_UTILS_H */

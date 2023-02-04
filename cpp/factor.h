#ifndef FACTOR_H
#define FACTOR_H

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

using ::std::vector;
using ::std::list;

class Factor {
public:
	vector<vector<char>> bundles;

	Factor(vector<vector<char>> b) {
		bundles = b;
	}

	Factor(int width, int num_features) {
		bundles = vector<vector<char>>(width, vector<char>(num_features));
	}

	Factor(){}

	// Comparison: + > - > 0 > * > #
	bool operator < (const Factor& other) const;

	bool operator > (const Factor& other) const;

	list<Factor> getNextFactors(
		const std::unordered_map<std::string, Factor>& alphabet, 
		int max_width, int max_features) const;

	// Returns a string displaying Factor contents. For debugging.
	std::string toString() const;
};

#endif /* FACTOR_H */

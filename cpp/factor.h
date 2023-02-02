#ifndef FACTOR_H
#define FACTOR_H

#include <vector>

using ::std::vector;

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

	void SetIsEdge(bool new_value) {
		if(new_value) {
			bundles.clear();
		}
		is_edge = new_value;
	}
private:
	bool is_edge = false;
};

#endif /* FACTOR_H */
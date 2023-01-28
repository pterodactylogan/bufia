#ifndef FACTOR_H
#define FACTOR_H

#include <vector>

using ::std::vector;

class Factor {
public:
	vector<vector<char>*> bundles;

	Factor(vector<vector<char>*> b) {
		bundles = b;
	}

	Factor(int width){
		bundles.reserve(width);
	}

	Factor() {}
};

#endif /* FACTOR_H */
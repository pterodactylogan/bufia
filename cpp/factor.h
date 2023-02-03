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

	// Comparison: + > - > 0 > # ; * > #
	bool operator < (const Factor& other) const {
		if(other.bundles.size() != bundles.size()) return false;
		for(int i=0; i<bundles.size(); i++) {
			for(int j=0; j<bundles.at(i).size(); j++) {
				if(other.bundles.at(i).size() != bundles.at(i).size()) return false;

				if(other.bundles[i][j] == '#' && bundles[i][j]!='#') return false;

				if(other.bundles[i][j] == '+' && 
					(bundles[i][j] == '-' || bundles[i][j] == '0'
						|| bundles[i][j] == '#')) return true;

				if(other.bundles[i][j] == '-'){
					if(bundles[i][j] == '0' || bundles[i][j] == '#') return true;
					if(bundles[i][j] == '+') return false;
				}

				if(other.bundles[i][j] == '0'){
					if(bundles[i][j] == '#') return true;
					if(bundles[i][j] == '+' || bundles[i][j] == '-') return false;
				}
			}
		}
		return false;
	}

	bool operator > (const Factor& other) const {
		if(other.bundles.size() != bundles.size()) return false;
		for(int i=0; i<bundles.size(); i++) {
			for(int j=0; j<bundles.at(i).size(); j++) {
				if(other.bundles.at(i).size() != bundles.at(i).size()) return false;

				if(other.bundles[i][j] == '#' && bundles[i][j]!='#') return true;

				if(other.bundles[i][j] == '0') {
					if(bundles[i][j] == '-' || bundles[i][j] == '+') return true;
					if(bundles[i][j] == '#') return false;
				}

				if(other.bundles[i][j] == '-') {
					if(bundles[i][j] == '+') return true;
					if(bundles[i][j] == '0' || bundles[i][j] == '#') return false;
				}

				if(other.bundles[i][j] == '+' && 
					(bundles[i][j] == '-' || bundles[i][j] == '0'
						|| bundles[i][j] == '#')) return false;
			}
		}
		return false;
	}

	std::string print_string() const {
		std::string result = "";
		for(const auto& bundle : bundles){
			result += "[";
			for(const auto& value : bundle) {
				result+= std::string(1, value) + ", ";
			}
			result +="]\n";
		}
		return result;
	}
};

#endif /* FACTOR_H */
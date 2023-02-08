#include <fstream>
#include <deque>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <memory>

#include "bufia_algorithm_utils.h"
#include "bufia_init_utils.h"
#include "factor.h"

using ::std::string;
using ::std::vector;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::pair;
using ::std::list;
using ::std::set;

int main(int argc, char **argv) {
	const int MAX_FACTOR_WIDTH = 3;
	const int MAX_FEATURES_PER_BUNDLE = 3;
	const int ABDUCTIVE_PRINCIPLE = 1;

	// load files from input flags
	if(argc < 2) {
		std::cout << "Expected at least two arguments: feature file and data file";
		return 1;
	}

	string feature_filename = argv[1];
	string data_filename = argv[2];
	std::ifstream feature_file (feature_filename);
	std::ifstream data_file (data_filename);

	if(!feature_file.is_open()) {
		std::cout << "Failed to open feature file";
		return 1;
	}
	if(!data_file.is_open()) {
		std::cout << "Failed to open data file";
		return 1;
	}

	vector<string> feature_order;
	// symbol -> width-1 Factor
	unordered_map<string, Factor> alphabet = LoadAlphabetFeatures(&feature_file, feature_order);

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet);

	list<Factor> queue = {Factor(vector<vector<char>>(1, 
		vector<char>(feature_order.size(), '*')))};
	vector<Factor> constraints;
	set<string> banned_ngrams;

	while(!queue.empty()) {
		Factor current = queue.front();
		queue.pop_front();
		if(Covers(constraints, current)) continue;

		if(Contains(positive_data[current.bundles.size()], current)) {
			list<Factor> next_factors = current.getNextFactors(alphabet, 
				MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);
			next_factors.remove_if([constraints](Factor fac){
				return Contains(constraints, fac);
			});

			queue.splice(queue.end(), next_factors);
		} else {
			if(ABDUCTIVE_PRINCIPLE == 0) {
				constraints.push_back(current);
				continue;
			}

			// if Abductive principle = 1, check whether constraint extends the grammar
			vector<string> ngrams = ComputeGeneratedNGrams(current, alphabet);
			bool added_ngrams = false;
			for(const auto& ngram : ngrams){
				bool redundant = false;
				// check all substrings to see if those are already banned
				for(int i = 1; i<ngram.size(); i++){
					if(redundant) break;
					for(int offset = 0; offset <= ngram.size()-i; offset++){
						// check if ngram[offset:offset+i] is in banned_ngrams
						string slice = ngram.substr(offset, i);
						if(banned_ngrams.find(slice) != banned_ngrams.end()) {
							redundant = true;
							break;
						}
					}
				}
				if(!redundant && banned_ngrams.insert(ngram).second == true) {
					added_ngrams = true;
				}
			}
			if(added_ngrams) {
				constraints.push_back(current);
			}
		}
	}

	for(Factor const& constraint : constraints){
		std::cout << Display(constraint, feature_order) << std::endl;
	}

	return 0;
}

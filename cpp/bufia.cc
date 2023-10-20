#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bufia_algorithm_utils.h"
#include "bufia_init_utils.h"
#include "factor.h"

using ::std::list;
using ::std::map;
using ::std::pair;
using ::std::set;
using ::std::string;
using ::std::unordered_map;
using ::std::unordered_set;
using ::std::vector;

double diff_timespec(struct timespec *time1, struct timespec *time0) {
	if(time1 == nullptr) {
		timespec t;
		time1 = &t;
		clock_gettime(CLOCK_REALTIME, time1);
	}
  return (time1->tv_sec - time0->tv_sec)
      + (time1->tv_nsec - time0->tv_nsec) / 1000000000.0;
}

int main(int argc, char **argv) {
	int MAX_FACTOR_WIDTH = 3;
	int MAX_FEATURES_PER_BUNDLE = 3;
	bool DEBUG_MODE = false;
	int ABDUCTIVE_PRINCIPLE = 1;
	int RANK_FEATURES = 1;
	string FEAT_DELIM = ",";
	bool ADD_WB = true;
	// should possibly be enum
	int ORDER = 1; // successor

	// STEP 1: read input flags
	if(argc < 3) {
		std::cout << "Expected at least two arguments: feature file and data file";
		return 1;
	}

	for(int i=3; i<argc; i++) {
		std::string arg = argv[i];

		std::size_t pos = arg.find("k=");
		if(pos != string::npos){
			MAX_FACTOR_WIDTH = std::stoi(arg.substr(pos+2));
			continue;
		}

		pos = arg.find("n=");
		if(pos != string::npos){
			MAX_FEATURES_PER_BUNDLE = std::stoi(arg.substr(pos+2));
			continue;
		}

		pos = arg.find("a=");
		if(pos != string::npos){
			ABDUCTIVE_PRINCIPLE = std::stoi(arg.substr(pos+2));
			continue;
		}

		pos = arg.find("o=");
		if(pos != string::npos) {
			ORDER = std::stoi(arg.substr(pos+2));
			continue;
		}

		pos = arg.find("f=");
		if(pos != string::npos) {
			RANK_FEATURES = std::stoi(arg.substr(pos+2));
			continue;
		}

		pos = arg.find("feat_delim=");
		if(pos != string::npos) {
			FEAT_DELIM = arg.substr(pos+11);
			continue;
		}

		pos = arg.find("wb=");
		if(pos != string::npos) {
			std::istringstream(arg.substr(pos+3)) >> std::boolalpha >> ADD_WB;
		}

		if(arg.find("debug") != string::npos) {
			DEBUG_MODE = true;
		}
	}

	string feature_filename = argv[2];
	string data_filename = argv[1];
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

	// STEP 2: Initialize alphabet and positive data
	timespec start_time;
	if(DEBUG_MODE) clock_gettime(CLOCK_REALTIME, &start_time);

	vector<string> feature_order;
	vector<std::pair<int, char>> feature_ranks;
	// symbol -> width-1 Factor
	unordered_map<string, Factor> alphabet = LoadAlphabetFeatures(&feature_file, 
		feature_order, feature_ranks, FEAT_DELIM, ADD_WB, RANK_FEATURES);
	const int NUM_FEAT = feature_order.size();

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet, ORDER, ADD_WB);

	if(DEBUG_MODE) {
		std::cout << "Time loading data (ns): " << 
		diff_timespec(nullptr, &start_time) << std::endl;
	}

	// STEP 3: BUFIA algorithm

	// start from length 1 factor with '*' for every feature (ie., universal matcher)
	Factor start = Factor(vector<vector<char>>(1, vector<char>(NUM_FEAT, '*')));
	vector<Factor> constraints;

	list<Factor> queue = {start};
	list<Factor> to_expand;

	while(true) {
		// if(DEBUG_MODE &&
		// 	(queue.size() + constraints.size() + to_expand.size()) % 1000 == 0) {
		// 	std::cout << "queue: " << queue.size() << std::endl;
		// 	std::cout << "constraints: " << constraints.size() << std::endl;
		// 	std::cout << "to_expand: " << to_expand.size() << std::endl;
		// }
		if(queue.empty()) {
			if(to_expand.empty()) break;
			else if(RANK_FEATURES){
				queue = to_expand.front().getNextFactors(alphabet,
					MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE, &feature_ranks);
				to_expand.pop_front();
			} else {
				queue = to_expand.front().getNextFactors(alphabet,
					MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE, nullptr);
				to_expand.pop_front();
			}
		}
		if(queue.empty()) continue;

		if(Covers(constraints, queue.front())) {
			queue.pop_front();
			continue;
		}

		// if something that matches this factor is in the positive data,
		// get all child factors and add to queue
		// otherwise, add to constraints
		bool contains = Contains(positive_data[queue.front().bundles.size()], queue.front());

		if(contains) {
			to_expand.push_back(queue.front());
		} else {
			constraints.push_back(queue.front());
		}
		queue.pop_front();
	}

	if(ABDUCTIVE_PRINCIPLE==1) {
	  // remove redundant constraints
	  set<vector<string>> banned_ngrams;
		
		for(Factor const& constraint : constraints){
			vector<vector<string>> ngrams = ComputeGeneratedNGrams(constraint, alphabet);
			bool added_ngrams = false;
			for(const auto& ngram : ngrams){
				bool redundant = false;
				// check all substrings to see if those are already banned
				for(int i = 1; i<ngram.size(); i++){
					if(redundant) break;
					for(int offset = 0; offset <= ngram.size()-i; offset++){
						// check if ngram[offset:offset+i] is in banned_ngrams
						vector<string> slice = 
							vector<string>(ngram.begin()+offset, ngram.begin() + offset + i);
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
				std::cout << Display(constraint, feature_order) << std::endl;
			}
		}
	} else {
		for(const auto& constraint : constraints) {
			std::cout << Display(constraint, feature_order) << std::endl;
		}
	}

	return 0;
}

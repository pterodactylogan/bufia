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
	int MAX_FACTOR_WIDTH = -1;
	int MAX_FEATURES_PER_BUNDLE = -1;
	int MAX_DISTANCE = -1;
	int IGNORE_COUNT = 0;
	bool DEBUG_MODE = false;
	// number of new kgrams which must be added by constraint
	int ABDUCTIVE_PRINCIPLE = 1;
	int RANK_FEATURES = 1;
	string FEAT_DELIM = ",";
	bool ADD_WB = false;
	// should possibly be enum
	int ORDER = 1; // successor
	vector<string> TIER;

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

		pos = arg.find("d=");
		if(pos != string::npos){
			MAX_DISTANCE = std::stoi(arg.substr(pos+2));
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
			continue;
		}

		pos = arg.find("t=");
		if(pos!= string::npos) {
			string symbols = arg.substr(pos+2);
			pos = 0;
			std::string symbol;
			while ((pos = symbols.find(",")) != std::string::npos) {
			    symbol = symbols.substr(0, pos);
			    TIER.push_back(symbol);
			    symbols.erase(0, pos + 1);
			}
			TIER.push_back(symbols);
			continue;
		}

		pos = arg.find("ignore=");
		if(pos != string::npos) {
			IGNORE_COUNT = std::stoi(arg.substr(pos+7));
			continue;
		}

		if(arg.find("debug") != string::npos) {
			DEBUG_MODE = true;
		}
	}

	if(MAX_DISTANCE == -1 && (MAX_FACTOR_WIDTH == -1 || MAX_FEATURES_PER_BUNDLE == -1)){
		MAX_DISTANCE = 9;
	}

	if(MAX_FACTOR_WIDTH == -1) {
		MAX_FACTOR_WIDTH = MAX_DISTANCE - 1;
	}

	if(!TIER.empty() && ADD_WB) TIER.push_back("#");

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
		feature_order, feature_ranks, FEAT_DELIM, ADD_WB, RANK_FEATURES, TIER);
	const int NUM_FEAT = feature_order.size();

	if(DEBUG_MODE) {
		std::cout << "Loaded Alphabet" << std::endl;
	}

	//  factor width -> vector of (factor, count) pairs
	unordered_map<int, vector<pair<Factor, int>>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet, TIER, ORDER, ADD_WB);

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

	// for debug mode only
	set<vector<string>> banned_ngrams;

	int loops = 0;
	while(true) {
		if(DEBUG_MODE && loops%1000 == 0) {
			std::cout << "q size: " << queue.size() <<std::endl;
		}
		if(queue.empty()) {
			if(to_expand.empty()) break;

			if(RANK_FEATURES == 1){
				queue = to_expand.front().getNextFactors(alphabet,
					MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE, MAX_DISTANCE, &feature_ranks);
			} else {
				queue = to_expand.front().getNextFactors(alphabet,
					MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE, MAX_DISTANCE, nullptr);
			}
			to_expand.pop_front();
		}
		if(queue.empty()) continue;

		if(Covers(constraints, queue.front(), ORDER)) {
			queue.pop_front();
			continue;
		}

		// if something that matches this factor is in the positive data,
		// get all child factors and add to queue
		// otherwise, add to constraints
		bool contains = Contains(positive_data[queue.front().bundles.size()], 
			queue.front(),
			IGNORE_COUNT);

		if(contains) {
			to_expand.push_back(queue.front());
		} else {
			auto constraint = queue.front();
			if(DEBUG_MODE) {
				if(ABDUCTIVE_PRINCIPLE > 0){
					set<vector<string>> ngrams = ComputeGeneratedNGrams(constraint, 
																																alphabet,
																																MAX_FACTOR_WIDTH);
					int new_ngrams = 0;
					for(const auto& ngram : ngrams){
						if(banned_ngrams.find(ngram) == banned_ngrams.end()) {
							++new_ngrams;
							if(new_ngrams >= ABDUCTIVE_PRINCIPLE) break;
						}
					}

					if(new_ngrams >= ABDUCTIVE_PRINCIPLE) {
						for(const auto& ngram : ngrams) {
							banned_ngrams.insert(ngram);
						}
						std::cout << Display(constraint, feature_order) << std::endl;
					}
				}
				else {
					std::cout << Display(constraint, feature_order) << std::endl;
				}
			}
			constraints.push_back(constraint);
		}
		queue.pop_front();

		if(DEBUG_MODE) ++loops;
	}

	if(DEBUG_MODE){
		return 0;
	}

	// STEP 4: abduction & print constraints
	if(ABDUCTIVE_PRINCIPLE > 0) {
		
		for(Factor const& constraint : constraints){
			set<vector<string>> ngrams = ComputeGeneratedNGrams(constraint, 
																															alphabet,
																															MAX_FACTOR_WIDTH);
			int new_ngrams = 0;
			for(const auto& ngram : ngrams){
				if(banned_ngrams.find(ngram) == banned_ngrams.end()) {
					++new_ngrams;
					if(new_ngrams >= ABDUCTIVE_PRINCIPLE) break;
				}
			}

			if(new_ngrams >= ABDUCTIVE_PRINCIPLE) {
				for(const auto& ngram : ngrams) {
					banned_ngrams.insert(ngram);
				}
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

#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
	// symbol -> width-1 Factor
	unordered_map<string, Factor> alphabet = LoadAlphabetFeatures(&feature_file, feature_order);

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet);

	if(DEBUG_MODE) {
		std::cout << "Time loading data (ns): " << 
		diff_timespec(nullptr, &start_time) << std::endl;
	}

	// STEP 3: BUFIA algorithm

	// start from length 1 factor with '*' for every feature (ie., universal matcher)
	list<Factor> queue = {Factor(vector<vector<char>>(1, 
		vector<char>(feature_order.size(), '*')))};
	vector<Factor> constraints;
	set<vector<string>> banned_ngrams;

	double total_covers = 0;
	double total_contains = 0;
	double total_extension_check = 0;
	double gen_ngrams_time = 0;
	double check_redundant_time = 0;

	while(!queue.empty()) {
		Factor current = queue.front();
		queue.pop_front();
		// if this factor is already covered by current constraints, skip
		timespec begin;
		if(DEBUG_MODE) clock_gettime(CLOCK_REALTIME, &begin);
		bool covers = Covers(constraints, current);
		if(DEBUG_MODE) total_covers += diff_timespec(nullptr, &begin);

		if(covers) continue;

		// if something that matches this factor is in the positive data,
		// get all applicable child factors and add to queue
		if(DEBUG_MODE) clock_gettime(CLOCK_REALTIME, &begin);
		bool contains = Contains(positive_data[current.bundles.size()], current);
		if(DEBUG_MODE) total_contains += diff_timespec(nullptr, &begin);

		if(contains) {
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
			if(DEBUG_MODE) clock_gettime(CLOCK_REALTIME, &begin);
			vector<vector<string>> ngrams = ComputeGeneratedNGrams(current, alphabet);
			timespec fin_ngrams_time;
			if(DEBUG_MODE) clock_gettime(CLOCK_REALTIME, &fin_ngrams_time);
			if(DEBUG_MODE) gen_ngrams_time += diff_timespec(&fin_ngrams_time, &begin);
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
			if(DEBUG_MODE) check_redundant_time += diff_timespec(nullptr, &fin_ngrams_time);
			if(DEBUG_MODE) total_extension_check += diff_timespec(nullptr, &begin);
			if(added_ngrams) {
				constraints.push_back(current);
			}
		}
	}

	if(DEBUG_MODE){
		std::cout << "Total seconds on Covers(): " << total_covers << std::endl;
		std::cout << "Total seconds on Contains(): " << total_contains << std::endl;
		std::cout << "Total seconds on extension checking: " << total_extension_check << std::endl;
		std::cout << "Time generating NGrams: " << gen_ngrams_time << std::endl;
		std::cout << "Time checking ngram redundancy: " << check_redundant_time << std::endl;
		std::cout << "Total time: " << diff_timespec(nullptr, &start_time) << std::endl;
	}

	for(Factor const& constraint : constraints){
		std::cout << Display(constraint, feature_order) << std::endl;
	}

	return 0;
}

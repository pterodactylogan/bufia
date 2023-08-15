#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mpi.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "bufia_algorithm_utils.h"
#include "bufia_init_utils.h"
#include "bufia_mpi_utils.h"
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
	int MPI_CHUNK_SIZE = 1;

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

		pos = arg.find("c=");
		if(pos != string::npos) {
			MPI_CHUNK_SIZE = std::stoi(arg.substr(pos+2));
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
	const int NUM_FEAT = feature_order.size();

	// factor width -> vector of factors
	unordered_map<int, vector<Factor>> positive_data = 
		LoadPositiveData(&data_file, MAX_FACTOR_WIDTH, alphabet);

	if(DEBUG_MODE) {
		std::cout << "Time loading data (ns): " << 
		diff_timespec(nullptr, &start_time) << std::endl;
	}

	// STEP 3: BUFIA algorithm

	// start from length 1 factor with '*' for every feature (ie., universal matcher)
	Factor start = Factor(vector<vector<char>>(1, vector<char>(NUM_FEAT, '*')));
	vector<Factor> constraints;

	// Begin MPI Manager/Worker structure

	std:: cout << "begin MPI" << std::endl;

	int rank, num_procs;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	if(rank==0) {
		list<Factor> queue = {start};
		// MANAGER flow
		map<int, list<Factor>::iterator> proc_locs;
		set<int> idle_procs;
		for(int i=1; i<num_procs; ++i) {
			idle_procs.insert(i);
		}

		bool done = false;
		auto curr = queue.begin();

		while(!done) {
			if(proc_locs.size() > 0) {
				// std::cout << "manager waiting on recieve." << std::endl;
				// Receive array of bools
				MPI_Status status;
				bool res[CHUNK_SIZE];
				MPI_Recv(&res, sizeof(res), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

				// find where in the queue that process is working
				auto loc = proc_locs[status.MPI_SOURCE];
				// for each factor, either add it to constraints, or add its next factors
				// as determined by boolean value
				for(int i=0; i<CHUNK_SIZE; ++i){
					if(res[i]) {
						list<Factor> next_factors = (*loc).getNextFactors(alphabet, 
							MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);
							queue.splice(queue.end(), next_factors);
					} else {
							constraints.push_back(*loc);
					}
					loc = queue.erase(loc);
				}
				proc_locs.erase(proc_locs.find(status.MPI_SOURCE));
				idle_procs.insert(status.MPI_SOURCE);
			}
			if(queue.size() == 0) break;
			if(curr ==  queue.end()) {
				continue;
			}
			if(queue.size() >= 3*CHUNK_SIZE && proc_locs.size() > 0) {
				// std::cout << "Manager: Queue size: " << queue.size() << std::endl;
				SendWork(constraints, CHUNK_SIZE, num_procs, MAX_FACTOR_WIDTH, NUM_FEAT, 
					queue, proc_locs, idle_procs, curr);
			} else {		
				// std::cout << "running processes: " << proc_locs.size() << std::endl;	
				// if this factor is already covered by current constraints, skip
				if(Covers(constraints, *curr)) {
					// std::cout << "covered, skipping" << std::endl;
					curr = queue.erase(curr);
					continue;
				}

				// if something that matches this factor is in the positive data,
				// get all child factors and add to queue
				// otherwise, add to constraints
				bool contains = Contains(positive_data[(*curr).bundles.size()], *curr);

				if(contains) {
					// std::cout << "adding children for: " << Display(*curr, feature_order) << std::endl;
					list<Factor> next_factors = (*curr).getNextFactors(alphabet, 
						MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);
					queue.splice(queue.end(), next_factors);
				} else {
					// std::cout << "adding constraint: " << Display(*curr, feature_order) << std::endl;
					constraints.push_back(*curr);
				}
				curr = queue.erase(curr);
			}
			if(proc_locs.size() == 0 && queue.empty()) done = true;
		}

		// send done message to workers
		// add enum for tags?
		char arr[CHUNK_SIZE][MAX_FACTOR_WIDTH][NUM_FEAT];
		for(int i=0; i<num_procs; ++i){
			// std::cout << "manager sending termination signal" << std::endl;
			MPI_Send(&arr, sizeof(arr), MPI_BYTE, i, /*tag=*/1, MPI_COMM_WORLD);
		}
	} else {
		// WORKER flow
		char arr[CHUNK_SIZE][MAX_FACTOR_WIDTH][NUM_FEAT];
		int offset = sizeof(arr[0][0]) / sizeof(arr[0][0][0]);
		while(true) {
			// std::cout << "worker " << rank << " waiting for work." << std::endl;
			// recieve work from manager
			MPI_Status status;
			MPI_Recv(&arr, sizeof(arr), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			// std::cout << "worker " << rank << " recieved work from manager" << std::endl;

			if(status.MPI_TAG == 1) break;

			bool res[CHUNK_SIZE];
			string chunks;
			
			// for factor in chunk
			for(int i=0; i<CHUNK_SIZE; ++i){
				vector<vector<char>> bundles;
				for(int j=0; j<MAX_FACTOR_WIDTH; ++j) {
					//std::cout << rank << ": " << arr[i][j][0] << std::endl;
					if(arr[i][j][0] == 0) break;
					bundles.push_back(vector<char>(arr[i][j], arr[i][j] + offset));
				}
				Factor fac(bundles);
				res[i] = Contains(positive_data[fac.bundles.size()], fac);
			}

			// Send result back
			MPI_Send(&res, sizeof(res), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
 	std::cout << "end MPI" << std::endl;

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

	return 0;
}

#include <ctime>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mpi.h>
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
	Factor start = Factor(vector<vector<char>>(1, vector<char>(feature_order.size(), '*')));
	list<Factor> queue = {start};
	vector<Factor> constraints;

	// Begin MPI Manager/Worker structure

	std:: cout << "begin MPI" << std::endl;

	int rank, num_procs;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	//std::cout << "I am " << rank << " of " << size << "\n";

	// MPI_Datatype dt_symb;
	// MPI_Type_contiguous(start.bundles.at(0).size(), MPI_CHAR, *dt_symb);

	if(rank==0) {
		// MANAGER flow
		set<int> active_procs;

		while(queue.size() < num_procs) {
			Factor current = queue.front();
			queue.pop_front();
			// if this factor is already covered by current constraints, skip
			bool covers = Covers(constraints, current);
			if(covers) continue;

			// if something that matches this factor is in the positive data,
			// get all applicable child factors and add to queue
			bool contains = Contains(positive_data[current.bundles.size()], current);

			if(contains) {
				list<Factor> next_factors = current.getNextFactors(alphabet, 
					MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);
				next_factors.remove_if([constraints](Factor fac){
					return Contains(constraints, fac);
				});

				queue.splice(queue.end(), next_factors);
			} else {
				constraints.push_back(current);
				continue;
			}
		}

		// send work to everyone, add all to active_procs
		for(int i=1; i<num_procs; ++i) {
			Factor a = queue.front();
			queue.pop_front();
			// convert Factor.bundles to array to send
			char arr[a.bundles.size()][a.bundles.at(0).size()];
			for(int i=0; i<a.bundles.size(); ++i) {
				std::copy(a.bundles.at(i).begin(), a.bundles.at(i).end(), arr[i]);
			}
			MPI_Send(&arr, sizeof(arr), MPI_BYTE, i, 0, MPI_COMM_WORLD);
			active_procs.insert(i);
		}

		bool done = false;

		while(!done) {
			//std::cout << "manager" << std::endl;
			// get results from any worker
			MPI_Status status;
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int bytes_amount;
			MPI_Get_count(&status, MPI_BYTE, &bytes_amount);
			int symb_size = sizeof(char[start.bundles.size()][start.bundles.at(0).size()]);
			char arr[bytes_amount/symb_size][start.bundles.at(0).size()];

			MPI_Recv(&arr, sizeof(arr), MPI_BYTE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
			active_procs.erase(status.MPI_SOURCE);
			//std::cout<< "manager, message from: " << status.MPI_SOURCE << std::endl;
			Factor fac(bytes_amount/symb_size, start.bundles.at(0).size());
			for(int i=0; i<fac.bundles.size(); ++i) {
				fac.bundles[i] = vector<char>(arr[i], arr[i] + (sizeof(arr[i])/ sizeof(arr[i][0])));
			}
			if(status.MPI_TAG == 0) {
				// fac not found in positive data. add as constraint.
				constraints.push_back(fac);
				//std::cout << "manager, constraint: " << Display(fac, feature_order) << std::endl;
				//if(constraints.size() > 10) done = true;
			} else {
				//std::cout << "manager, found: " << Display(fac, feature_order) << std::endl;
				// found. add next factors to queue.
				list<Factor> next_factors = fac.getNextFactors(alphabet, 
				MAX_FACTOR_WIDTH, MAX_FEATURES_PER_BUNDLE);

				queue.splice(queue.end(), next_factors);
			}

			if(queue.empty()) {
				if(active_procs.empty()) done = true; 
				continue;
			}

			// send next n in queue to whatever process finished
			Factor q = queue.front();
			queue.pop_front();
			while(Covers(constraints, q)) {
				q = queue.front();
				queue.pop_front();
				if(queue.empty()) {
					if(active_procs.empty()) done = true; 
					break;
				}
			}

			if(Covers(constraints, q)) continue;

			char q_arr[q.bundles.size()][q.bundles.at(0).size()];
			// convert Factor.bundles to array to send
			for(int i=0; i<q.bundles.size(); ++i) {
				std::copy(q.bundles.at(i).begin(), q.bundles.at(i).end(), q_arr[i]);
			}
			//std::cout << "manager, sending: " << Display(q, feature_order) << std::endl;
			MPI_Send(&q_arr, sizeof(q_arr), MPI_BYTE, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
			active_procs.insert(status.MPI_SOURCE);
		}

		// send done message to workers
		// add enum for tags?
		char arr[1][start.bundles.at(0).size()];
		for(int i=0; i<num_procs; ++i){
			MPI_Send(&arr, sizeof(arr), MPI_BYTE, i, /*tag=*/1, MPI_COMM_WORLD);
		}
	} else {
		// WORKER flow
		bool done = false;
		while(!done) {
			//std::cout << "worker" << std::endl;
			// recieve work from manager
			MPI_Status status;
			MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			int bytes_amount;
			MPI_Get_count(&status, MPI_BYTE, &bytes_amount);
			int symb_size = sizeof(char[start.bundles.size()][start.bundles.at(0).size()]);
			char arr[bytes_amount/symb_size][start.bundles.at(0).size()];

			MPI_Recv(&arr, sizeof(arr), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			if(status.MPI_TAG == 1) break;



			// Convert bundle array back into Factor
			Factor fac(bytes_amount/symb_size, start.bundles.at(0).size());
			for(int i=0; i<fac.bundles.size(); ++i) {
				fac.bundles[i] = vector<char>(arr[i], arr[i] + (sizeof(arr[i])/ sizeof(arr[i][0])));
			}
			//std::cout << "rank: " << rank << " factor: " << fac.toString() << std::endl;

			// compute result
			if(Contains(positive_data[fac.bundles.size()], fac)) {
				//std::cout << "rank: " << rank << " found fac: " << Display(fac, feature_order) << std::endl;
				MPI_Send(&arr, sizeof(arr), MPI_BYTE, 0, /*tag=*/1, MPI_COMM_WORLD);
			} else {
				//std::cout << "rank: " << rank << " banned fac: " << Display(fac, feature_order) << std::endl;
				MPI_Send(&arr, sizeof(arr), MPI_BYTE, 0, /*tag=*/0, MPI_COMM_WORLD);
			}
		}
	}

	MPI_Finalize();
  // End MPI section
	// set<vector<string>> banned_ngrams;

	
	for(Factor const& constraint : constraints){
		std::cout << Display(constraint, feature_order) << std::endl;
	}

	return 0;
}

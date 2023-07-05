#include "bufia_mpi_utils.h"

#include "bufia_algorithm_utils.h"

void SendWork(const vector<Factor>& constraints, int chunk_size, int num_procs,
	int max_width, int num_feat,
	list<Factor>& queue, map<int, list<Factor>::iterator>& proc_locs, 
	set<int>& idle_procs, list<Factor>::iterator& start) {
		auto curr = start;
		while(!idle_procs.empty()) {
			int i = *idle_procs.begin();
			//std::cout<< i<< std::endl;
			int packaged = 0;
			char chunk[chunk_size][max_width][num_feat];
			while(packaged < chunk_size) {
				if(Covers(constraints, *curr)) {
					curr = queue.erase(curr);
				} else {
					if(proc_locs.find(i) == proc_locs.end()) {
						proc_locs[i] = curr;
					}
					for(int i=0; i<max_width; ++i) {
						for(int j=0; j<num_feat; ++j) {
							if(i< (*curr).bundles.size()) {
								chunk[packaged][i][j] = (*curr).bundles[i][j];
								//std::cout << (*curr).bundles[i][j] << std::endl;
								//std::cout << chunk[packaged][i][j] << std::endl;
							} else {
								chunk[packaged][i][j] = 0; // null for all values in extra bundles
							}
						}
					}
					++packaged;
					++curr;
				}

				if(curr == queue.end()) break;
			}

			if (packaged == chunk_size) {
				// send completed package
				MPI_Send(&chunk, sizeof(chunk), MPI_BYTE, i, 0, MPI_COMM_WORLD);
				idle_procs.erase(idle_procs.find(i));
				start = curr;
			} else {
				proc_locs.erase(proc_locs.find(i));
			}
			if(curr == queue.end()) break;
		}
}
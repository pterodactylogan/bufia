#ifndef BUFIA_MPI_UTILS_H
#define BUFIA_MPI_UTILS_H

#include <list>
#include <map>
#include <mpi.h>
#include <set>
#include <vector>

#include "factor.h"

using ::std::list;
using ::std::map;
using ::std::set;
using ::std::vector;

void SendWork(const vector<Factor>& constraints, int chunk_size, int num_procs, 
	int max_width, int num_feat, list<Factor>& queue, 
	map<int, list<Factor>::iterator>& proc_locs, set<int>& idle_procs,
	list<Factor>::iterator& start);

#endif /* BUFIA_ALGORITHM_UTILS_H */
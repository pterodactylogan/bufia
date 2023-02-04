#include "bufia_algorithm_utils.h"

#include <omp.h>
#include <iostream>
#include <set>
#include <unordered_map>

#include "factor.h"

using ::std::vector;

bool Contains(const vector<Factor>& positive_data,
	const Factor& parent) {
	bool found = false;
	#pragma omp parallel for shared(found)
		for(int i=0; i<positive_data.size(); i++){
			// TODO: is there a way to actually make all threads stop?
			if(found) continue;

			if(parent.generates(positive_data[i])){
				found = true;
			}
		}
	return found;
}
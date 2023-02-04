#ifndef BUFIA_ALGORITHM_UTILS_H
#define BUFIA_ALGORITHM_UTILS_H

#include <set>
#include <unordered_map>

#include "factor.h"

// Returns true if any Factor in `positive_data` can be generated from `parent`
// false otherwise.
bool Contains(const std::vector<Factor>& positive_data,
	const Factor& parent);

#endif /* BUFIA_ALGORITHM_UTILS_H */
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <omp.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "factor.h"
#include "bufia_algorithm_utils.h"

using ::std::list;
using ::std::vector;
using ::std::string;

// THIS FILE IS FOR DEVELOPMENT ONLY
// NOT A PROPERLY SET UP TEST SUITE
void TestGenerates() {		
	vector<vector<char>> vec = {{'*', '+', '+', '*', '-'}, {'*', '+', '*', '-', '*'}};
	Factor fac(vec);

	vec = {{'*', '+', '+', '*', '-'}, {'+', '+', '0', '-', '*'}};
	Factor child(vec);

	vec = {{'*', '*', '+', '*', '-'}, {'*', '+', '*', '-', '*'}};
	Factor non_child1(vec);

	vec = {{'*', '+', '+', '*', '-'}, {'*', '-', '*', '-', '*'}};
	Factor non_child2(vec);

	vec = {{'*', '+', '+', '*', '-'}};
	Factor fac2(vec);

	vec = {{'#', '#', '#', '#', '#'}};
	Factor edge(vec);

	std::cout << fac.generates(child) << std::endl; // exp T
	std::cout << fac.generates(non_child1) << std::endl; // exp F
	std::cout << fac.generates(non_child2) << std::endl; // exp F
	std::cout << fac2.generates(edge) << std::endl; // exp F

	Factor cons1({{'#', '#'}, {'*', '*'}});
	Factor cons2({{'*', '*'}, {'#', '#'}});

	Factor sm_child({{'#', '#'}, {'+', '*'}});

	std::cout << cons1.generates(sm_child) << std::endl; // exp T
}

void TestNextFactors() {	
	vector<vector<char>> vec = {{'*', '+', '+', '*', '-'}, {'*', '+', '*', '-', '*'}};
	Factor fac(vec);

	std::unordered_map<std::string, Factor> map;
	map["a"] = Factor({{'+', '+', '-', '-', '-'}});
	list<Factor> li = fac.getNextFactors(map, 3, 3);
	std::cout << li.size() << std::endl;
	for(const auto& item : li){
		std::cout << item.toString();
		std::cout << std::endl;
	}

}

void TestCompare() {	
	vector<vector<char>> parent_b = {{'*', '+'}, {'*', '*'}};
	Factor parent(parent_b);

	vector<vector<char>> bc_1 = {{'0', '0', '0', '+', '+', '+', '-', '-'}};
	Factor child_1(bc_1);

	vector<vector<char>> bc_2 = {{'0', '0', '0', '+', '-', '+', '+', '-'}};
	Factor child_2(bc_2);

	std::cout << (parent < child_1) << std::endl; // expect: F
	std::cout << (parent > child_1) << std::endl; // exp: F
	std::cout << (child_1 > child_2) << std::endl; // exp: T
	std::cout << (child_1 < child_2) << std::endl; // exp: F
}

void TestCovers() {
	Factor cons1({{'#', '#'}, {'*', '*'}});
	Factor cons2({{'*', '*'}, {'#', '#'}});

	Factor child({{'#', '#'}, {'+', '*'}});

	std::cout << Covers({cons1, cons2}, child); // exp T
}

void TestComputeGeneratedNGrams() {
	std::unordered_map<std::string, Factor> alphabet({
		{"a", Factor({{'+', '+'}})},
		{"b", Factor({{'+', '-'}})},
		{"c", Factor({{'-', '-'}})}
	});

	Factor fac({{'*', '-'}, {'+', '*'}, {'*', '*'}});

	vector<string> ngrams = ComputeGeneratedNGrams(fac, alphabet);
	std::cout << ngrams.size() << std::endl; // exp 12

	for(const auto& ngram : ngrams){
		std::cout << ngram << ", ";
	}
	std::cout << std::endl;
}

int main(int argc, char **argv) {
	//TestNextFactors();
	//TestGenerates();
	//TestCovers();
	TestComputeGeneratedNGrams();
}
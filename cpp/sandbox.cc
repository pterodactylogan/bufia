#include <ctime>
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

	vec = {{'#', '#'}, {'*', '+'}, {'#', '#'}};
	Factor bounded(vec);

	vec = {{'#', '#'}, {'*', '+'}, {'+', '+'}, {'*', '*'}};
	Factor nc_4(vec);

	std::cout << "Expect F: " << bounded.generates(nc_4) <<std::endl;

	vec = {{'#', '#'}, {'*', '+'}};
	Factor l_bounded(vec);

	vec = {{'#', '#'}, {'-', '+'}, {'+', '+'}};
	Factor lb_child(vec);

	vec = {{'*', '+'}, {'#', '#'}};
	Factor r_bounded(vec);

	vec = {{'-', '+'}, {'+', '+'}, {'#', '#'}};
	Factor rb_child(vec);

	vec = {{'-', '+'}, {'+', '-'}, {'#', '#'}};
	Factor rb_nc(vec);

	std::cout << "Expect T: " << l_bounded.generates(lb_child) << std::endl;
	std::cout << "Expect T: " << r_bounded.generates(rb_child) << std::endl;
	std::cout << "Expect F: " << r_bounded.generates(rb_nc) << std::endl;

	vec = {{'-', '*'}, {'*', '+'}};
	Factor unbounded(vec);

	vec = {{'+', '*'}, {'-', '+'}, {'+', '+'}, {'-', '-'}};
	Factor ub_child(vec);

	vec = {{'+', '*'}, {'-', '+'}, {'+', '-'}, {'-', '-'}};
	Factor ub_nc(vec);

	std::cout << "Expect T: " << unbounded.generates(ub_child) << std::endl;
	std::cout << "Expect F: " << unbounded.generates(ub_nc) << std::endl;
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

	Factor fac1({{'*', '-'}, {'+', '*'}, {'*', '*'}});

	vector<vector<string>> ngrams1 = ComputeGeneratedNGrams(fac1, alphabet);
	std::cout << ngrams1.size() << std::endl; // exp 12

	for(const auto& ngram : ngrams1){
		for(const auto& symbol : ngram) {
			std::cout << symbol << " ";
		}
		std::cout << ", ";
	}
	std::cout << std::endl;

	Factor fac2({{'-', '-'}, {'+', '*'}});

	std::set<vector<string>> ngrams2 = ComputeGeneratedNGrams(fac2, alphabet, 3);
	std::cout << ngrams2.size() << std::endl; // exp 12

	// all 3-grams containing "ca" or "cb"
	for(const auto& ngram : ngrams2){
		for(const auto& symbol : ngram) {
			std::cout << symbol << " ";
		}
		std::cout << ", ";
	}
	std::cout << std::endl;

	Factor fac3({{'-', '-'}, {'-', '-'}});

	std::set<vector<string>> ngrams3 = ComputeGeneratedNGrams(fac3, alphabet, 4);
	std::cout << ngrams3.size() << std::endl; // exp 21

	// all 4-grams containing "cc"
	for(const auto& ngram : ngrams3){
		for(const auto& symbol : ngram) {
			std::cout << symbol << " ";
		}
		std::cout << ", ";
	}
	std::cout << std::endl;
}


int main(int argc, char **argv) {
	//TestGenerates();
	//TestNextFactors();
	//TestCompare();
	//TestCovers();
	TestComputeGeneratedNGrams();
}
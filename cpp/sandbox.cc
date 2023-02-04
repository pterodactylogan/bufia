#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "factor.h"

using ::std::list;
using ::std::vector;

// THIS FILE IS FOR DEVELOPMENT ONLY
// NOT A PROPERLY SET UP TEST SUITE

void TestNextFactors() {	
	vector<vector<char>> vec = {{'*', '+', '+', '*', '-'}, {'*', '+', '*', '-', '*'}};
	Factor fac(vec);

	std::unordered_map<std::string, Factor> map;
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

int main(int argc, char **argv) {
	TestNextFactors();
}
/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++11 -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <chrono>

#include "Board.h"
#include "solver.h"
#include "PreCalculate.h"

int main(int argc, const char* argv[]) {
	// Uncomment this to use read and write to file
//    freopen("sample-input.txt", "r", stdin);
//    freopen("output.txt", "w", stdout);
	std::cout << "Threes-Project3: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
	InitLookUpTables();
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
//	std::cout << "InitLookUpTables duration:" << duration << std::endl;

	std::string solve_args;
	int precision = 10;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0) {
			solve_args = para.substr(para.find("=") + 1);
		} else if (para.find("--precision=") == 0) {
			precision = std::stol(para.substr(para.find("=") + 1));
		}
	}

	t1 = std::chrono::high_resolution_clock::now();
	solver solve(solve_args);
	t2 = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
//	std::cout << "Explore tree duration:" << duration << std::endl;

	Board state;
	state_type type;
	int hint;
	std::cout << std::setprecision(precision);
	while (std::cin >> type >> state >> hint) {
		auto value = solve.solve(state, hint, type);
		std::cout << type << " " << state << " +" << hint;
		if(std::get<0>(value) == INT32_MAX) {
            std::cout << " = -1" << std::endl;
		}
		else std::cout << " = " << std::get<0>(value) << " " << std::get<1>(value) << " " << std::get<2>(value) << std::endl;
	}

	return 0;
}

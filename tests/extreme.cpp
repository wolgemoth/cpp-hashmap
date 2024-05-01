#include "../Hashmap.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <thread>

/**
 * @file extreme.cpp
 * @brief Extreme tests for the functionality of the hashmap.
 */
int main([[maybe_unused]] int _argc, [[maybe_unused]] char* _argv[]) {
	
	LouiEriksson::Hashmap<int, std::string> hashmap;

	std::cout << "~ EXTREME TESTS ~\n";
	
	// Test 1: The grind.
	{
		std::cout << "Test 1: The grind..." << std::flush;
	
		static constexpr int iterations = 200000;
		static constexpr int concurrency = 100;
		
		std::vector<std::pair<std::thread, std::exception_ptr>> threads;
		
		// Perform concurrent deletions, insertions, overwrites, and reads.
		for (int i = 0; i < concurrency; ++i) {
			
			std::exception_ptr exceptionPtr;
			
			threads.emplace_back(
				std::thread([i, &threads, &hashmap, &exceptionPtr]() {
					
					try {
						for (int j = 0; j < iterations; ++j) {
							
							hashmap.Remove(j);
							hashmap.Add(j, std::to_string(j));
							hashmap.Assign(j, std::to_string(j));
							
							if (auto item = hashmap.Get(j)) {
								assert(item.value() == std::to_string(j) && "Item value mismatch!");
							}
						}
					}
					catch (...) {
						threads[i].second = std::current_exception();
					}
				}),
				nullptr
			);
		}
		
		// Throw any exceptions:
		for (auto& thread : threads) {
			
			thread.first.join();
			
			if (thread.second) {
				std::rethrow_exception(thread.second);
			}
		}
		
		// Assert the hashmap has the correct size.
		assert(hashmap.size() == iterations && "Erroneous insertion detected!");
		
		// Clear hashmap and assert it is empty.
		hashmap.Clear();
		assert(hashmap.empty() && "Clearing failed!");
		
		std::cout << "Done.\n";
	}
	
	std::cout << "All tests passed!" << std::endl;
	
	return 0;
}
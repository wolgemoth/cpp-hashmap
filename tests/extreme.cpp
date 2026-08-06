#include "../hashmap.hpp"

#include <iostream>
#include <cassert>
#include <mutex>
#include <string>
#include <thread>

/**
 * @file extreme.cpp
 * @brief Extreme tests for the functionality of the hashmap.
 */
int main([[maybe_unused]] int _argc, [[maybe_unused]] char* _argv[]) {
	
	louieriksson::hashmap<int, std::string> hashmap;

	std::cout << "~ EXTREME TESTS ~\n";
	
	// Test 1: The grind.
	{
		std::cout << "Test 1: The grind..." << std::flush;
	
		static constexpr int iterations = 200000;
		static constexpr int concurrency = 100;
		
		std::vector<std::pair<std::thread, std::exception_ptr>> threads;
		
		// The hashmap only guarantees safety for individual operations. As documented, compound
		// sequences of operations (such as a remove-add-assign-get sequence performed on the same
		// key) must be synchronised externally by the caller to avoid a reference returned by
		// get() being invalidated by another thread's concurrent mutation before it is read.
		std::mutex compound_op_lock;
		
		// Perform concurrent deletions, insertions, overwrites, and reads.
		for (int i = 0; i < concurrency; ++i) {
			
			std::exception_ptr exceptionPtr;
			
			threads.emplace_back(
				std::thread([i, &threads, &hashmap, &exceptionPtr, &compound_op_lock]() {
					
					try {
						for (int j = 0; j < iterations; ++j) {
							
							const std::lock_guard<std::mutex> lock(compound_op_lock);
							
							hashmap.remove(j);
							hashmap.add(j, std::to_string(j));
							hashmap.assign(j, std::to_string(j));
							
							if (auto item = hashmap.get(j)) {
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
		hashmap.clear();
		assert(hashmap.empty() && "Clearing failed!");
		
		std::cout << "Done.\n";
	}
	
	std::cout << "All tests passed!" << std::endl;
	
	return 0;
}
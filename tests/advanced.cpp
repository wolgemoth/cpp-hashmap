#include "../Hashmap.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <thread>

/**
 * @file advanced.cpp
 * @brief Advanced tests for the functionality of the hashmap.
 */
int main([[maybe_unused]] int _argc, [[maybe_unused]] char* _argv[]) {
	
	LouiEriksson::Hashmap<int, std::string> hashmap;

	std::cout << "~ ADVANCED TESTS ~\n";
	
	// Test 1: Large volume of data
	{
		std::cout << "Test 1: Large volume of data..." << std::flush;
		
		static constexpr int iterations = 2000000;
		
		for (int i = 0; i < iterations; ++i) {
			hashmap.Add(i, std::to_string(i));
		}
		assert(hashmap.size() == iterations);
		
		std::cout << "Done.\n";
	}
	
	// Test 2: Duplicate keys
	{
		std::cout << "Test 2: Duplicate keys..." << std::flush;
		
		const auto controlSize = hashmap.size();
		
		for (int i = 0; i < controlSize; ++i) {
			hashmap.Add(i, "Duplicate");
			
			assert((hashmap.Get(i).value() != "Duplicate") &&     "Duplicate found.");
			assert((hashmap.size() == controlSize)         && "Erroneous insertion.");
		}
		
		std::cout << "Done.\n";
	}
	
	// Test 3: Key collision
	{
		std::cout << "Test 3: Key collision..." << std::flush;
		
		static constexpr int iterations = 2000000;
		
		for (int i = 1; i < iterations; ++i) {
			hashmap.Add(-i, "Negative");
			
			assert(hashmap.Get(-i).value() == "Negative");
		}
		
		std::cout << "Done.\n";
	}
	
	// Test 4: High churn
	{
		std::cout << "Test 4: High churn..." << std::flush;
		
		hashmap.Clear();
		
		static constexpr int iterations = 2000000;
		
		for (int i = iterations; i < iterations * 2; ++i) {
			hashmap.Add(i, std::to_string(i));
			hashmap.Remove(i - iterations);
		}
		assert(hashmap.size() == iterations);
		
		std::cout << "Done.\n";
	}
	
	// Test 5: Concurrency
	{
		std::cout << "Test 5: Concurrency..." << std::flush;
		
		hashmap.Clear();
		
		static constexpr int iterations = 2000000;
		
		std::exception_ptr writerException = nullptr;
		std::exception_ptr readerException = nullptr;
		
		std::thread writer([&hashmap, &writerException]() {
			
			try {
				for (int i = 0; i < iterations; ++i) {
					hashmap.Add(i, std::to_string(i));
				}
			}
			catch (...) {
				writerException = std::current_exception();
			}
		});
		
		std::thread reader([&hashmap, &readerException]() {
			
			try {
				int last_size = 0;
				
				while (last_size < iterations) {
					if (hashmap.size() > last_size) {
						for (int i = last_size; i < hashmap.size(); ++i) {
							assert((hashmap.Get(i).value() == std::to_string(i)));
						}
						last_size = hashmap.size();
					}
				}
			}
			catch (...) {
				readerException = std::current_exception();
			}
		});
		
		writer.join();
		reader.join();
		
		if (writerException) { std::rethrow_exception(writerException); }
		if (readerException) { std::rethrow_exception(readerException); }

		assert(hashmap.size() == iterations);
		
		std::cout << "Done.\n";
	}
	
	std::cout << "All tests passed!" << std::endl;
	
	return 0;
}
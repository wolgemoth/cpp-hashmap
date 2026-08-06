#include "../hashmap.hpp"

#include <iostream>
#include <cassert>
#include <string>

/**
 * @file basic.cpp
 * @brief Basic tests for the functionality of the hashmap.
 */
int main([[maybe_unused]] int _argc, [[maybe_unused]] char* _argv[]) {
	
	louieriksson::hashmap<int, std::string> hashmap;
	
	std::cout << "~ BASIC TESTS ~\n";
	
	// Test 1: Insertion
	{
		std::cout << "Test 1: Insertion..." << std::flush;
		
		hashmap.add(1, "One");
		hashmap.add(2, "Two");
		hashmap.add(3, "Three");
		
		std::cout << "Done.\n";
	}
	
	// Test 2: Existence check
	{
		std::cout << "Test 2: Existence check..." << std::flush;
		
		assert(hashmap.contains_key(1) && "Failed on key 1.");
		assert(hashmap.contains_key(2) && "Failed on key 2.");
		assert(hashmap.contains_key(3) && "Failed on key 3.");
		
		std::cout << "Done.\n";
	}
	
	// Test 3: Value retrieval
	{
		std::cout << "Test 3: Value retrieval..." << std::flush;
		
		auto item1 = hashmap.get(1);
		auto item2 = hashmap.get(2);
		auto item3 = hashmap.get(3);
		
		assert((item1.value() == "One"  ) && "Failed on key 1.");
		assert((item2.value() == "Two"  ) && "Failed on key 2.");
		assert((item3.value() == "Three") && "Failed on key 3.");
		
		std::cout << "Done.\n";
	}
	
	// Test 4: Overwriting
	{
		std::cout << "Test 4: Overwriting..." << std::flush;
		
		hashmap.assign(1, "New One");
		
		auto item1 = hashmap.get(1);
		
		assert((item1.value() == "New One") && "Failed on key 1.");
		
		std::cout << "Done.\n";
	}
	
	// Test 5: Deletion
	{
		std::cout << "Test 5: Deletion..." << std::flush;
	
		hashmap.remove(1);
		
		assert(!hashmap.contains_key(1) && "Failed on key 1.");
		
		std::cout << "Done.\n";
	}
	
	// Test 6: Clearing
	{
		std::cout << "Test 5: Clearing..." << std::flush;
		
		hashmap.clear();
		
		assert(hashmap.size() == 0 && "Failed on key 1.");
		
		std::cout << "Done.\n";
	}
	
	std::cout << "All tests passed!\n";
	
	return 0;
}
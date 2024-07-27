# C++ Hashmap (2.3.0)

## Table of Contents

- [About](#About)
- [Instructions](#Instructions)
- [Dependencies](#Dependencies)
- [References](#References)

### About

This is a hashmap written in C++.

It has a similar API to C#'s [Dictionary](https://learn.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2?view=net-8.0)  and self-initializes like an [std::vector](https://en.cppreference.com/w/cpp/container/vector). Currently, it uses [sequential chaining](https://en.wikipedia.org/wiki/Hash_table#Separate_chaining) for collision resolution. More collision resolution techniques may be added in the future.

This structure provides robust exception safety, and is suitable for use in a concurrent environment. Furthermore, it supports move semantics and initialiser lists.

Explicit finalization of the hashmap is not necessary. However, if you are storing manually-managed memory, then remember to free any elements before removal.

If you find a bug or have a feature-request, please raise an issue.

### Instructions

The implementation is header-only and written in templated C++17. You should not need to make any adjustments to your project settings or compiler flags. 

Simply include it in your project and you are ready to start!

#### Example:
    
    #include <string>
    
    #include "Hashmap.hpp"
    
    LouiEriksson::Hashmap<std::string, float> hashmap {
        { "key1", 1.0f },
        { "key2", 2.0f },
        { "key3", 3.0f },
    }

    int main() {

        if (const auto item = hashmap.Get("key3")) {
            std::cout << "Value: " << item.value() << '\n';
        }
        else {
            std::cout << "Key not in Hashmap!\n";
        }

        return 0;
    }

### Dependencies

The hashmap was written in C++17 and utilises the following standard headers:

#### &lt;algorithm&gt;
#### &lt;cstddef&gt;
#### &lt;functional&gt;
#### &lt;initializer_list&gt;
#### &lt;iostream&gt;
#### &lt;mutex&gt;
#### &lt;optional&gt;
#### &lt;stdexcept&gt;
#### &lt;vector&gt;

### Why not use &lt;unordered_map&gt;?

I find unordered_map to be way too verbose for most situations.

In this implementation, key existence and value retrieval are merged into a single conditional expression. This allows for simpler, cleaner code that affords better exception handling.

### Note

Please note that while the hashmap is capable of being used in a concurrent environment, it does not provide a mechanism for synchronising changes to the hashmap which are made in between operations.

Therefore, if you need to perform a synchronous series of operations on the Hashmap while it is being used in a concurrent context, you should lock access to the hashmap to one thread while doing so, otherwise you may encounter race conditions.

### References

- Wang, Q. (Harry) (2020). Implementing Your Own HashMap (Explanation + Code). YouTube. Available at: https://www.youtube.com/watch?v=_Q-eNqTOxlE [Accessed 2021].

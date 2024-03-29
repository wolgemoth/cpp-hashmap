# C++ Hashmap (1.0.4)

## Table of Contents

- [About](#About)
- [Instructions](#Instructions)
- [Dependencies](#Dependencies)
- [References](#References)

### About

This is a hashmap written in C++.

It has a similar API to C#'s [Dictionary](https://learn.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2?view=net-8.0)  and self-initializes like an [std::vector](https://en.cppreference.com/w/cpp/container/vector). Currently, it uses [sequential chaining](https://en.wikipedia.org/wiki/Hash_table#Separate_chaining) for collision resolution. More collision resolution techniques may be added in the future.

Explicit finalization of the hashmap is not necessary. However, if you are storing manually-managed memory, then remember to free any elements before removal.

I have taken precautions to improve the exception safety of the hashmap, although it hasn't been fully stress-tested. Please be aware that since this implementation uses [std::vector](https://en.cppreference.com/w/cpp/container/vector) for the container, anything that breaks a [std::vector](https://en.cppreference.com/w/cpp/container/vector) will also break the hashmap.

If you find a bug or have a feature-request, please raise an issue.

### Instructions

This implementation is header-only. Simply include it in your project and you are ready to start.

#### Example:
    
    #include <string>
    
    #include "Hashmap.h"
    
    Hashmap<std::string, float> hashmap;

    int main() {

        hashmap.Add("item1", 1.0f);
        hashmap.Add("item2", 2.0f);
        hashmap.Add("item3", 3.0f);

        float item;
        if (hashmap.Get("item3", item)) {
            std::cout << "item3: " << item << '\n';
        }
        else {
            std::cout << "item3 not in Hashmap!\n";
        }

        return 0;
    }

### Dependencies

The hashmap was written in C++17 and utilises the following standard headers:

#### &lt;cstddef&gt;
This header is used for [size_t](https://en.cppreference.com/w/c/types/size_t), which is used to represent the size of the container.

#### &lt;functional&gt;
This header is used for [std::hash](https://en.cppreference.com/w/cpp/utility/hash), which allows for generic hashing of C++ types. Developers can also provide custom specializations for their own types.

#### &lt;stdexcept&gt;
This header is used for [std::runtime_error](https://en.cppreference.com/w/cpp/error/runtime_error), which enables the script to throw meaningful exceptions.

#### &lt;vector&gt;
This header is used for [std::vector](https://en.cppreference.com/w/cpp/container/vector), which serves as the resizable container for the hashmap.

### References

- Wang, Q. (Harry) (2020). Implementing Your Own HashMap (Explanation + Code). YouTube. Available at: https://www.youtube.com/watch?v=_Q-eNqTOxlE [Accessed 2021].

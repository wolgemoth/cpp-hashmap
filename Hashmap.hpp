/*
 * MIT License
 *
 * Copyright (c) 2024 Louis Eriksson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef LOUIERIKSSON_HASHMAP_HPP
#define LOUIERIKSSON_HASHMAP_HPP

#define HASHMAP_SUPPRESS_EXCEPTION_WARNING // Uncomment if you wish to remove the warning about possible unhandled exceptions.

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <exception>
#include <vector>

namespace louieriksson {

	/**
	 * @mainpage Version 2.4.0
	 * @details Custom Hashmap implementation accepting a customisable key and value type.
	 *          This implementation requires that your "key" type is compatible with std::hash and that the stored data types are copyable.
	 * @see Wang, Q. (Harry) (2020). Implementing Your Own HashMap (Explanation + Code). YouTube.
	 *      Available at: https://www.youtube.com/watch?v=_Q-eNqTOxlE [Accessed 2021].
	 * @tparam Tk Key type of the Hashmap.
	 * @tparam Tv Value type of the Hashmap.
	 */
	template<typename Tk, typename Tv>
	class hashmap final {

    private:

		/**
		 * @brief Returns a reference to the Hashmap's mutex.
		 */
		static std::recursive_mutex& s_lock() {
			static std::recursive_mutex lock;
			return lock;
		}

	public:

		/**
		 * @brief Represents a key-value pair.
		 *
		 * This struct is used to store a key-value pair, where 'Tk' represents the type of the key and 'Tv' represents the type of the value.
		 */
		struct kvp final {

			Tk first;
			Tv second;

			kvp(const Tk& _key, const Tv& _value) :
				 first(_key),
				second(_value) {}

			constexpr kvp(const kvp& _other) :
				 first(_other.first),
				second(_other.second) {}

			kvp(kvp&& _rhs)  noexcept :
                 first(std::move(_rhs.first)),
                second(std::move(_rhs.second)) {}

			kvp& operator = (const kvp& _other) {
				if (this != &_other) {
					 first = _other.first;
					second = _other.second;
				}
				return *this;
			}

			kvp& operator = (kvp&& _other)  noexcept {
				if (this != &_other) {
					 first = std::move(_other.first);
					second = std::move(_other.second);
				}
				return *this;
			}
		};

	private:

		/** @brief Buckets of the Hashmap. */
		std::vector<std::vector<kvp>> m_buckets;

		/** @brief Current number of elements within the Hashmap. */
		size_t m_size;

		/**
		 * @brief Calculate the hashcode of a given object using std::hash.
		 * @param[in] _item Item to calculate hash of.
		 * @return Hashcode of _item.
		 * @throw std::exception If the type of _item is not supported by std::hash.
		 */
		static constexpr size_t get_hashcode(const Tk& _item) {
			return std::hash<Tk>()(_item);
		}

		/**
		 * @brief Reinitialise the Hashmap. An expensive operation that increases the Hashmap's capacity.
		 *
		 * @param _newSize The new size of the Hashmap.
		 */
		constexpr void resize(const size_t& _newSize) {

			std::vector<std::vector<kvp>> shallow_cpy(m_buckets);

			m_size = 0U;
			m_buckets.clear();
			m_buckets.resize(_newSize > 0U ? _newSize : 1U);

			for (auto& bucket : shallow_cpy) {
				for (auto& kvp : bucket) {
                    assign(std::move(kvp.first), std::move(kvp.second));
				}
			}
		}

		/**
		 * @brief Retrieves a reference to the entry within the Hashmap with the given key, if one exists.
		 * This method will throw an exception if no entry is found. Consider using get() for safe access instead.
		 *
		 * @param[in] _key Key of the entry to retrieve.
		 * @return Out value result.
		 * @throw std::runtime_error If no entry is found.
		 * @see get(const Tk& _key, Tv& _out)
		 */
		constexpr const Tv& get_or_throw(const Tk& _key) const {

			const Tv* result = nullptr;

			if (!m_buckets.empty()) {

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				size_t hash = get_hashcode(_key);
				size_t i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				for (auto& kvp : bucket) {

					if (get_hashcode(kvp.first) == hash) {
						result = &kvp.second;

						break;
					}
				}
			}

			if (result == nullptr) {
				throw std::runtime_error("Attempted to access a nonexistent entry from the Hashmap.");
			}

			return *result;
		}

	public:

		/**
		 * @brief Initialise Hashmap.
		 * @param[in] _capacity Initial capacity of the Hashmap. Must be larger than 0.
		 */
		constexpr hashmap(const size_t& _capacity = 1U) : m_size(0U) { // NOLINT(*-explicit-constructor)
			m_buckets.resize(_capacity);
		}

		/**
		 * @brief Initialise Hashmap using a collection of key-value pairs.
		 * @details Please note: The provided collection should be distinct. Otherwise, some data loss may occur as duplicate entries will be ignored.
		 *
		 * @param[in] _items A collection of key-value pairs.
		 * @param[in] _capacity Initial capacity of the Hashmap. If a value less than 1 is assigned, it will use the size of the provided collection.
		 */
		[[maybe_unused]] constexpr hashmap(const std::initializer_list<kvp>& _items, const size_t& _capacity = 0U) : m_size(0U) {

			size_t auto_capacity = _capacity;

			if (auto_capacity < 1) {
				auto_capacity = std::max<size_t>(_items.size(), 1);
			}

			m_buckets.resize(auto_capacity);

			for (const auto& item : _items) {
                assign(item.first, item.second);
			}
		}

		struct optional_ref final {

			friend hashmap;

		private:

			using optional_t = std::optional<std::reference_wrapper<const Tv>>;

			const optional_t m_optional;

			explicit optional_ref(optional_t&& _optional) : m_optional(_optional) {};

		public:

			                 [[nodiscard]] const Tv& value()                 const { return m_optional.value(); }
            [[maybe_unused]] [[nodiscard]] const Tv& value_or(const Tv&& _t) const { return m_optional.value_or(_t); }

			[[nodiscard]] bool has_value() const { return m_optional.has_value(); }

			[[nodiscard]] const Tv& operator  *() const { return  value(); }
			[[nodiscard]] const Tv* operator ->() const { return &value(); }

			[[nodiscard]] operator bool() const { return has_value(); } // NOLINT(*-explicit-constructor)
		};

		/**
		 * @brief Returns the number of items stored within the Hashmap.
		 * @return The number of items stored within the Hashmap.
		 */
		[[nodiscard]] const size_t& size() const noexcept {

  			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			return m_size;
		}

		/**
		 * @brief Is the Hashmap empty?
		 * @return Returns true if the Hashmap contains no entries.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return size() == 0;
		}

		/**
		 * @brief Queries for the existence of an item in the Hashmap.
		 *
		 * @param[in] _key Key of the entry.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 * @return True if successful, false otherwise.
		 */
        [[maybe_unused]] bool contains_key(const Tk& _key, [[maybe_unused]] std::exception_ptr _exception = nullptr) const noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			auto result = false;

			try {

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				size_t hash = get_hashcode(_key);
				size_t i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				for (auto& kvp : bucket) {

					if (get_hashcode(kvp.first) == hash) {
						result = true;

						break;
					}
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}

			return result;
		}

		/**
		 * @brief Inserts a new entry into the Hashmap with given key and value, if one does not already exist.
		 * If you are trying to modify an existing key, see Hashmap::assign.
		 *
		 * @param[in] _key Key of the entry.
		 * @param[in] _value Value of the entry.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 * @return True if successful, false otherwise.
		 */
		bool add(const Tk& _key, const Tv& _value, [[maybe_unused]] std::exception_ptr _exception = nullptr) noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			auto result = true;

			try {

				if (m_size >= m_buckets.size()) {
                    resize(m_buckets.size() * 2);
				}

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				const auto hash = get_hashcode(_key);
				const auto i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				// In the case of a hash collision, determine if the key is unique.
				// We will treat duplicate insertions as a mistake on the developer's part and return failure.
				for (auto& kvp : bucket) {
					if (get_hashcode(kvp.first) == hash) {
						result = false;

						break;
					}
				}

				// Insert the item into the bucket.
				if (result) {
					m_size++;

					bucket.emplace_back(_key, _value);
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}

			return result;
		}

		/**
		 * @brief Inserts a new entry into the Hashmap with given key and value using move semantics, if one does not already exist.
		 * If you are trying to modify an existing key, see Hashmap::assign.
		 *
		 * @param[in] _key Key of the entry.
		 * @param[in] _value Value of the entry.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 * @return True if successful, false otherwise.
		 */
		bool add(const Tk&& _key, const Tv&& _value, [[maybe_unused]] std::exception_ptr _exception = nullptr) noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			auto result = true;

			try {

				if (m_size >= m_buckets.size()) {
                    resize(m_buckets.size() * 2);
				}

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				const auto hash = get_hashcode(_key);
				const auto i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				// In the case of a hash collision, determine if the key is unique.
				// We will treat duplicate insertions as a mistake on the developer's part and return failure.
				for (auto& kvp : bucket) {
					if (get_hashcode(kvp.first) == hash) {
						result = false;

						break;
					}
				}

				// Insert the item into the bucket.
				if (result) {
					m_size++;

					bucket.emplace_back(_key, _value);
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}

			return result;
		}

		/**
		 * @brief Inserts or replaces an entry within the Hashmap with the given key.
		 *
		 * @param[in] _key Key of the entry.
		 * @param[in] _value Value of the entry.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 */
		void assign(const Tk& _key, const Tv& _value, [[maybe_unused]] std::exception_ptr _exception = nullptr) noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			try {

				if (m_size >= m_buckets.size()) {
                    resize(m_buckets.size() * 2);
				}

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				const auto hash = get_hashcode(_key);
				const auto i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				auto exists = false;
				for (auto& kvp : bucket) {

					if (get_hashcode(kvp.first) == hash) {
						exists = true;

						kvp.second = _value;

						break;
					}
				}

				if (!exists) {
					m_size++;

					bucket.emplace_back(_key, _value);
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}
		}

		/**
		 * @brief Inserts or replaces an entry within the Hashmap with the given key using move semantics.
		 *
		 * @param[in] _key Key of the entry.
		 * @param[in] _value Value of the entry.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 */
		void assign(Tk&& _key, Tv&& _value, [[maybe_unused]] std::exception_ptr _exception = nullptr) noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			try {

				if (m_size >= m_buckets.size()) {
                    resize(m_buckets.size() * 2);
				}

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				const auto hash = get_hashcode(_key);
				const auto i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				auto exists = false;
				for (auto& kvp : bucket) {

					if (get_hashcode(kvp.first) == hash) {
						exists = true;

						kvp.second = std::move(_value);

						break;
					}
				}

				if (!exists) {
					m_size++;

					bucket.emplace_back(std::move(_key), std::move(_value));
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}
		}

		/**
		 * @brief Removes entry with given key from the Hashmap.
		 *
		 * @param[in] _key Key of the entry to be removed.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 * @return True if successful, false otherwise.
		 */
		bool remove(const Tk& _key, [[maybe_unused]] std::exception_ptr _exception = nullptr) noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			bool result = false;

			try {

				// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
				const size_t hash = get_hashcode(_key);
				const size_t i = hash % m_buckets.size();

				auto& bucket = m_buckets[i];

				// In the case of accessing a "collided" hash, find the value in the bucket using equality checks.
				for (auto itr = bucket.begin(); itr < bucket.end(); itr++) {

					if (get_hashcode(itr->first) == hash) {
						result = true;

						bucket.erase(itr);

						break;
					}
				}

				m_size -= static_cast<size_t>(result);
			}
			catch (...) {
				_exception = std::current_exception();
			}

			return result;
		}

		/**
		 * @brief Retrieves the value associated with the given key from the hash table.
		 *
		 * @tparam Tk The type of the key.
		 * @param[in] _key The key to retrieve the value for.
		 * @param[out] _exception (optional) A pointer to any exception caught during the operation.
		 * @return An optional reference to the value associated with the key, or std::nullopt if the key is not present.
		 * @note This function is noexcept.
		 */
		optional_ref get(const Tk& _key, std::exception_ptr _exception = nullptr) const noexcept {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			typename optional_ref::optional_t result = std::nullopt;

			try {

				if (!m_buckets.empty()) {

					// Create an index by taking the key's hash value and "wrapping" it with the number of buckets.
					size_t hash = get_hashcode(_key);
					size_t i = hash % m_buckets.size();

					auto& bucket = m_buckets[i];

					for (auto& kvp : bucket) {

						if (get_hashcode(kvp.first) == hash) {
							result = std::cref(kvp.second);
							break;
						}
					}
				}
			}
			catch (...) {
				_exception = std::current_exception();
			}

			return optional_ref(std::move(result));
		}

		/**
		 * @brief Trims unused entries from the end of the Hashmap.
		 */
        [[maybe_unused]] void trim() {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			size_t trimStart = 1U;

			for (size_t i = trimStart; i < m_buckets.size(); ++i) {
				if (m_buckets[i].size() != 0U) {
					trimStart = i + 1U;
				}
			}

			if (trimStart < m_buckets.size()) {
				m_buckets.erase(m_buckets.begin() + trimStart);
			}
		}

		/**
		 * @brief Returns a shallow copy of all entries stored within the Hashmap.
		 * @return A shallow copy of all entries stored within the Hashmap.
		 */
        [[maybe_unused]] [[nodiscard]] std::vector<Tk> keys() const {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			std::vector<Tk> result;

			for (const auto& bucket : m_buckets) {
				for (const auto& kvp : bucket) {
					result.emplace_back(kvp.first);
				}
			}

			return result;
		}

		/**
		 * @brief Returns a shallow copy of all entries stored within the Hashmap.
		 * @return A shallow copy of all entries stored within the Hashmap.
		 */
		[[nodiscard]] std::vector<Tv> values() const {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			std::vector<Tv> result;

			for (const auto& bucket : m_buckets) {
				for (const auto& kvp : bucket) {
					result.emplace_back(kvp.second);
				}
			}

			return result;
		}

		/**
		 * @brief Returns a shallow copy of all entries stored within the Hashmap.
		 * @return A shallow copy of all entries stored within the Hashmap.
		 */
        [[maybe_unused]] [[nodiscard]] std::vector<kvp> get_all() const {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			std::vector<kvp> result;

			for (const auto& bucket : m_buckets) {
				for (const auto& kvp : bucket) {
					result.emplace_back(kvp);
				}
			}

			return result;
		}

		/**
		 * @brief Reserves memory for the container to have a minimum capacity of _newSize elements.
		 *
		 * @param[in] _new_size The minimum capacity to reserve for the container.
		 */
        [[maybe_unused]] void reserve(const std::size_t& _new_size) {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			if (m_size < _new_size) {
                resize(_new_size);
			}
		}

		/**
		 * @brief Clears all entries from the Hashmap.
		 */
		void clear() noexcept {

			const std::lock_guard<std::recursive_mutex> lock(s_lock());

			try {
				m_buckets.clear();
				m_size = 0U;
			}
			catch (const std::exception& e) {
				std::cerr << e.what() << "\n";
			}
		}

		/**
		 * @brief Retrieves a reference to the entry within the Hashmap with the given key, if one exists.
		 * This method will throw an exception if no entry is found. Consider using get() for safe access instead.
		 *
		 * @param[in] _key Key of the entry to retrieve.
		 * @return Out value result.
		 * @throw std::runtime_error If no entry is found.
		 *
		 * @see get(const Tk& _key, Tv& _out)
		 */
#ifndef HASHMAP_SUPPRESS_EXCEPTION_WARNING
		[[deprecated("This function does not guarantee exception-safety and will explicitly throws if no entry exists. Consider using get() if exception-safe access is required.\nSuppress this warning by defining \"HASHMAP_SUPPRESS_UNSAFE_WARNING\".")]]
#endif
		const Tv& operator[](const Tk& _key) const {

   			const std::lock_guard<std::recursive_mutex> lock(s_lock());

		    return get_or_throw(_key);
		}

		/* ITERATORS */

		/**
		 * @class const_iterator
		 * @brief Represents an iterator to traverse through the elements in a Hashmap.
		 */
		class const_iterator final {

			friend hashmap;

			using outer_itr = typename std::vector<std::vector<kvp>>::const_iterator;
			using inner_itr = typename std::vector<kvp>::const_iterator;

		private:

			outer_itr m_outer;
			outer_itr m_outer_end;
			inner_itr m_inner;

			constexpr const_iterator(const outer_itr& _outer,
			                         const outer_itr& _outer_end,
			                         const inner_itr& _inner) :
				    m_outer(_outer),
				m_outer_end(_outer_end),
				    m_inner(_inner)
			{
				if (m_outer != m_outer_end && m_inner == m_outer->end()) {
					++(*this);
				}
			}

		public:

			const const_iterator& operator ++() {

				if (++m_inner == m_outer->end()) {

					while (++m_outer != m_outer_end) {

						if (!m_outer->empty()) {
							m_inner = m_outer->begin();
							break;
						}
					}
					if (m_outer == m_outer_end) {
						m_inner = inner_itr();
					}
				}
				return *this;
			}

			const kvp& operator *() const { return *m_inner; }

			bool operator ==(const const_iterator& other) const { return ((m_outer == other.m_outer) && (m_outer == m_outer_end || m_inner == other.m_inner)); }
			bool operator !=(const const_iterator& other) const { return !operator ==(other); }
		};

		constexpr const_iterator begin() const { return const_iterator(m_buckets.begin(), m_buckets.end(), m_buckets.empty() ? typename std::vector<kvp>::const_iterator() : m_buckets.begin()->begin()); }
		constexpr const_iterator   end() const { return const_iterator(m_buckets.end(),   m_buckets.end(), typename std::vector<kvp>::const_iterator()); }
	};

} // louieriksson

#endif //LOUIERIKSSON_HASHMAP_HPP

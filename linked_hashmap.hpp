/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is order in which keys were inserted into map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	typedef pair<const Key, T> value_type;

private:
	struct Node {
		value_type *data;
		Node *prev;
		Node *next;
		Node *bucket_next;
		Node *bucket_prev;

		Node() : data(nullptr), prev(nullptr), next(nullptr), bucket_next(nullptr), bucket_prev(nullptr) {}
		Node(const Key &k, const T &v) : data(new value_type(k, v)), prev(nullptr), next(nullptr), bucket_next(nullptr), bucket_prev(nullptr) {}
		Node(const value_type &v) : data(new value_type(v)), prev(nullptr), next(nullptr), bucket_next(nullptr), bucket_prev(nullptr) {}
		~Node() { delete data; }
	};

	Node **buckets;
	size_t bucket_count;
	size_t num_elements;
	Node *head;
	Node *tail;
	Hash hash_func;
	Equal equal_func;

	static const size_t DEFAULT_CAPACITY = 16;
	static const double LOAD_FACTOR;

	void init() {
		buckets = new Node*[bucket_count];
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
		head = new Node();
		tail = new Node();
		head->next = tail;
		tail->prev = head;
		num_elements = 0;
	}

	void clear_nodes() {
		Node *curr = head->next;
		while (curr != tail) {
			Node *tmp = curr;
			curr = curr->next;
			delete tmp;
		}
		head->next = tail;
		tail->prev = head;
		num_elements = 0;
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
	}

	void rehash(size_t new_capacity) {
		Node **new_buckets = new Node*[new_capacity];
		for (size_t i = 0; i < new_capacity; ++i) {
			new_buckets[i] = nullptr;
		}

		Node *curr = head->next;
		while (curr != tail) {
			size_t index = hash_func(curr->data->first) % new_capacity;
			curr->bucket_next = new_buckets[index];
			curr->bucket_prev = nullptr;
			if (new_buckets[index]) {
				new_buckets[index]->bucket_prev = curr;
			}
			new_buckets[index] = curr;
			curr = curr->next;
		}

		delete[] buckets;
		buckets = new_buckets;
		bucket_count = new_capacity;
	}

	void check_expand() {
		if (num_elements >= bucket_count * LOAD_FACTOR) {
			rehash(bucket_count * 2);
		}
	}

public:
	class const_iterator;
	class iterator {
	private:
		Node *node;
		linked_hashmap *container;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;

		iterator() : node(nullptr), container(nullptr) {}
		iterator(Node *n, linked_hashmap *c) : node(n), container(c) {}
		iterator(const iterator &other) : node(other.node), container(other.container) {}

		iterator operator++(int) {
			iterator tmp = *this;
			++(*this);
			return tmp;
		}

		iterator & operator++() {
			if (node == nullptr || node == container->tail) {
				throw invalid_iterator();
			}
			node = node->next;
			return *this;
		}

		iterator operator--(int) {
			iterator tmp = *this;
			--(*this);
			return tmp;
		}

		iterator & operator--() {
			if (node == nullptr || node == container->head->next) {
				throw invalid_iterator();
			}
			node = node->prev;
			return *this;
		}

		value_type & operator*() const {
			if (node == nullptr || node == container->head || node == container->tail) {
				throw invalid_iterator();
			}
			return *(node->data);
		}

		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}

		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node;
		}

		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}

		bool operator!=(const const_iterator &rhs) const {
			return node != rhs.node;
		}

		value_type* operator->() const noexcept {
			return node->data;
		}

		friend class linked_hashmap;
		friend class const_iterator;
	};

	class const_iterator {
	private:
		Node *node;
		const linked_hashmap *container;

	public:
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::output_iterator_tag;

		const_iterator() : node(nullptr), container(nullptr) {}
		const_iterator(Node *n, const linked_hashmap *c) : node(n), container(c) {}
		const_iterator(const const_iterator &other) : node(other.node), container(other.container) {}
		const_iterator(const iterator &other) : node(other.node), container(other.container) {}

		const_iterator operator++(int) {
			const_iterator tmp = *this;
			++(*this);
			return tmp;
		}

		const_iterator & operator++() {
			if (node == nullptr || node == container->tail) {
				throw invalid_iterator();
			}
			node = node->next;
			return *this;
		}

		const_iterator operator--(int) {
			const_iterator tmp = *this;
			--(*this);
			return tmp;
		}

		const_iterator & operator--() {
			if (node == nullptr || node == container->head->next) {
				throw invalid_iterator();
			}
			node = node->prev;
			return *this;
		}

		const value_type & operator*() const {
			if (node == nullptr || node == container->head || node == container->tail) {
				throw invalid_iterator();
			}
			return *(node->data);
		}

		bool operator==(const iterator &rhs) const {
			return node == rhs.node;
		}

		bool operator==(const const_iterator &rhs) const {
			return node == rhs.node;
		}

		bool operator!=(const iterator &rhs) const {
			return node != rhs.node;
		}

		bool operator!=(const const_iterator &rhs) const {
			return node != rhs.node;
		}

		const value_type* operator->() const noexcept {
			return node->data;
		}

		friend class linked_hashmap;
	};

	linked_hashmap() : bucket_count(DEFAULT_CAPACITY) {
		init();
	}

	linked_hashmap(const linked_hashmap &other) : bucket_count(other.bucket_count), hash_func(other.hash_func), equal_func(other.equal_func) {
		init();
		Node *curr = other.head->next;
		while (curr != other.tail) {
			insert(*(curr->data));
			curr = curr->next;
		}
	}

	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this == &other) return *this;

		clear_nodes();

		Node *curr = other.head->next;
		while (curr != other.tail) {
			insert(*(curr->data));
			curr = curr->next;
		}

		return *this;
	}

	~linked_hashmap() {
		clear_nodes();
		delete head;
		delete tail;
		delete[] buckets;
	}

	T & at(const Key &key) {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return curr->data->second;
			}
			curr = curr->bucket_next;
		}
		throw index_out_of_bound();
	}

	const T & at(const Key &key) const {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return curr->data->second;
			}
			curr = curr->bucket_next;
		}
		throw index_out_of_bound();
	}

	T & operator[](const Key &key) {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return curr->data->second;
			}
			curr = curr->bucket_next;
		}

		check_expand();
		index = hash_func(key) % bucket_count;
		Node *new_node = new Node(key, T());

		new_node->bucket_next = buckets[index];
		new_node->bucket_prev = nullptr;
		if (buckets[index]) {
			buckets[index]->bucket_prev = new_node;
		}
		buckets[index] = new_node;

		new_node->prev = tail->prev;
		new_node->next = tail;
		tail->prev->next = new_node;
		tail->prev = new_node;

		num_elements++;
		return new_node->data->second;
	}

	const T & operator[](const Key &key) const {
		return at(key);
	}

	iterator begin() {
		return iterator(head->next, this);
	}

	const_iterator cbegin() const {
		return const_iterator(head->next, this);
	}

	iterator end() {
		return iterator(tail, this);
	}

	const_iterator cend() const {
		return const_iterator(tail, this);
	}

	bool empty() const {
		return num_elements == 0;
	}

	size_t size() const {
		return num_elements;
	}

	void clear() {
		clear_nodes();
	}

	pair<iterator, bool> insert(const value_type &value) {
		size_t index = hash_func(value.first) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, value.first)) {
				return pair<iterator, bool>(iterator(curr, this), false);
			}
			curr = curr->bucket_next;
		}

		check_expand();
		index = hash_func(value.first) % bucket_count;
		Node *new_node = new Node(value);

		new_node->bucket_next = buckets[index];
		new_node->bucket_prev = nullptr;
		if (buckets[index]) {
			buckets[index]->bucket_prev = new_node;
		}
		buckets[index] = new_node;

		new_node->prev = tail->prev;
		new_node->next = tail;
		tail->prev->next = new_node;
		tail->prev = new_node;

		num_elements++;
		return pair<iterator, bool>(iterator(new_node, this), true);
	}

	void erase(iterator pos) {
		if (pos.node == nullptr || pos.node == head || pos.node == tail || pos.container != this) {
			throw invalid_iterator();
		}

		Node *node = pos.node;

		size_t index = hash_func(node->data->first) % bucket_count;
		if (node->bucket_prev) {
			node->bucket_prev->bucket_next = node->bucket_next;
		} else {
			buckets[index] = node->bucket_next;
		}
		if (node->bucket_next) {
			node->bucket_next->bucket_prev = node->bucket_prev;
		}

		node->prev->next = node->next;
		node->next->prev = node->prev;

		delete node;
		num_elements--;
	}

	size_t count(const Key &key) const {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return 1;
			}
			curr = curr->bucket_next;
		}
		return 0;
	}

	iterator find(const Key &key) {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return iterator(curr, this);
			}
			curr = curr->bucket_next;
		}
		return end();
	}

	const_iterator find(const Key &key) const {
		size_t index = hash_func(key) % bucket_count;
		Node *curr = buckets[index];
		while (curr != nullptr) {
			if (equal_func(curr->data->first, key)) {
				return const_iterator(curr, this);
			}
			curr = curr->bucket_next;
		}
		return cend();
	}
};

template<class Key, class T, class Hash, class Equal>
const double linked_hashmap<Key, T, Hash, Equal>::LOAD_FACTOR = 0.75;

}

#endif

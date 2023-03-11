/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "../KernelTest.h"
#include <kernel/kstd/map.hpp>
#include <kernel/random.h>

int num_constructed = 0;
int num_destructed = 0;

class TestValue {
public:
	TestValue() {
		num_constructed++;
	}
	TestValue(const TestValue&) {
		num_constructed++;
	}
	~TestValue() {
		num_destructed++;
	}
};

using TestMap = kstd::map<int, TestValue>;
using IntMap = kstd::map<int, int>;
using IntPairVector = kstd::vector<kstd::pair<int, int>>;

KERNEL_TEST(map_insert) {
	IntMap map;
	for(int i = 0; i < 1000; i++) {
		map[i] = i * 2;
	}
	ENSURE_EQ(map.size(), 1000);
	for(int i = 0; i < 1000; i++) {
		ENSURE_EQ(map[i], i*2);
	}
}

KERNEL_TEST(map_remove) {
	IntMap map;
	for(int i = 0; i < 1000; i++) {
		map[i] = i * 2;
	}
	ENSURE_EQ(map.size(), 1000);
	for(int i = 0; i < 1000; i += 2) {
		map.erase(i);
	}
	ENSURE_EQ(map.size(), 500);
	for(int i = 0; i < 1000; i ++) {
		if(i % 2) {
			ENSURE_EQ(map[i], i*2);
		} else {
			ENSURE(!map.contains(i));
		}
	}
}

KERNEL_TEST(map_constructors_destructors) {
	num_constructed = 0;
	num_destructed = 0;
	{
		TestMap map;
		// Populate map
		for (int i = 0; i < 1000; i++) {
			map[i] = TestValue();
		}
		ENSURE_EQ(map.size(), 1000);
		// Erase half
		for (int i = 0; i < 1000; i+=2) {
			map.erase(i);
		}
		ENSURE_EQ(map.size(), 500);
		// Overwrite all
		for (int i = 0; i < 1000; i++) {
			map[i] = TestValue();
		}
		ENSURE_EQ(map.size(), 1000);
	}
	ENSURE_EQ(num_constructed, num_destructed);
}

void randomly_populate(IntMap& map, IntPairVector& in_map) {
	for(int i = 0; i < 1000; i++) {
		int a;
		do {
			a = rand();
		} while(map.contains(a));
		int b = rand();
		map[a] = b;
		in_map.push_back({a, b});
	}
}

void check_values(IntMap& map, IntPairVector& in_map) {
	ENSURE_EQ(map.size(), in_map.size());
	for(int i = 0; i < in_map.size(); i++) {
    	int a = in_map[i].first;
		int b = in_map[i].second;
		ENSURE_EQ(map[a], b);
	}
}

KERNEL_TEST(map_random_insert_remove) {
	IntMap map;
	IntPairVector in_map; // A vector containing the pairs that should be in the map

	// Randomly populate and check values are correct
	randomly_populate(map, in_map);
	check_values(map, in_map);

	// Remove half of the values randomly and check again
	for(int i = 0; i < in_map.size() / 2; i++) {
		int idx = rand() % in_map.size();
		map.erase(in_map[idx].first);
		in_map.erase(idx);
	}
	check_values(map, in_map);

	// Populate map and check values again
	randomly_populate(map, in_map);
	check_values(map, in_map);
}

KERNEL_TEST(map_iterator) {
	IntMap map;
	IntPairVector in_map;
	randomly_populate(map, in_map);

	// Test forward iterator
	size_t count = 0;
	int last;
	for(auto val : map) {
		if(count)
			ENSURE(val.first > last);
		count++;
		last = val.first;
	}

	// Test backward iterator
	count = 0;
	auto iter = map.end();
	while(iter != map.begin()) {
		iter--;
		if(count)
			ENSURE(iter->first < last);
		count++;
		last = iter->first;
	}

	ENSURE_EQ(count, map.size());
}

int calculate_height(kstd::map<int,int>::Node* node) {
	int left_height = node->left ? calculate_height(node->left) : 0;
	int right_height = node->right ? calculate_height(node->right) : 0;
	return max(left_height, right_height) + 1;
}

KERNEL_TEST(map_height_and_balance) {
	IntMap map;
	IntPairVector in_map;
	randomly_populate(map, in_map);

	auto check_balance = [&]() {
		for(auto val : map) {
			auto node = map.find_node(val.first);
			ENSURE(node);
			ENSURE_EQ(node->height, calculate_height(node));
			ENSURE(node->balance_factor() > -2 && node->balance_factor() < 2);
		}
	};

	// Ensure the initial tree is properly balanced
	check_balance();

	// Remove half of the values randomly and check again
	for(int i = 0; i < in_map.size() / 2; i++) {
		int idx = rand() % in_map.size();
		map.erase(in_map[idx].first);
		in_map.erase(idx);
	}
	check_balance();
}
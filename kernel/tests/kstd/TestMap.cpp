/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "../KernelTest.h"
#include <kernel/kstd/map.hpp>

int num_constructed = 0;
int num_destructed = 0;

class TestValue {
public:
	TestValue() {
		num_constructed++;
	}
	~TestValue() {
		num_destructed++;
	}
};

using TestMap = kstd::map<int, TestValue>;
using IntMap = kstd::map<int, int>;

KERNEL_TEST(insert) {
	IntMap map;
	for(int i = 0; i < 1000; i++) {
		map[i] = i * 2;
	}
	for(int i = 0; i < 1000; i++) {
		ENSURE_EQ(map[i], i*2);
	}
}

KERNEL_TEST(remove) {
	IntMap map;
	for(int i = 0; i < 1000; i++) {
		map[i] = i * 2;
	}
	for(int i = 0; i < 1000; i += 2) {
		map.erase(i);
	}
	for(int i = 0; i < 1000; i ++) {
		if(i % 2) {
			ENSURE_EQ(map[i], i*2);
		} else {
			ENSURE(!map.contains(i));
		}
	}
}

KERNEL_TEST(constructors_destructors) {
	num_constructed = 0;
	num_destructed = 0;
	{
		TestMap map;
		for (int i = 0; i < 1000; i++) {
			map[i] = TestValue();
		}
		for (int i = 0; i < 1000; i++) {
			map.erase(i);
		}
	}
	ENSURE_EQ(num_constructed, num_destructed);
}
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "../KernelTest.h"
#include "../../kstd/SharedPtr.h"

class TestClass: public kstd::Shared<TestClass> {
public:
	static int num_alloced;

	TestClass() {
		num_alloced++;
	}

	~TestClass() {
		num_alloced--;
	}
};

int TestClass::num_alloced = 0;

KERNEL_TEST(basic_shared_ptr) {
	TestClass::num_alloced = 0;
	kstd::vector<kstd::SharedPtr<TestClass>> vec;
	for(int i = 0; i < 1000; i++)
		vec.push_back(kstd::make_shared<TestClass>());
	ENSURE_EQ(TestClass::num_alloced, 1000);
	for(int i = 0; i < 1000; i++)
		vec.erase(0);
	ENSURE_EQ(TestClass::num_alloced, 0);
}

KERNEL_TEST(basic_weak_ptr) {
	TestClass::num_alloced = 0;
	kstd::vector<kstd::SharedPtr<TestClass>> vec;
	kstd::vector<kstd::WeakPtr<TestClass>> weak_vec;

	for(int i = 0; i < 1000; i++) {
		auto test = kstd::make_shared<TestClass>();
		vec.push_back(test);
		weak_vec.push_back(test);
	}

	ENSURE_EQ(TestClass::num_alloced, 1000);

	for(int i = 0; i < 1000; i++) {
		ENSURE_EQ(vec[0].ref_count()->weak_count(), 2);
		ENSURE_EQ(vec[0].ref_count()->strong_count(), 1);
		vec.erase(0);
	}

	ENSURE_EQ(TestClass::num_alloced, 0);

	for(int i = 0; i < 1000; i++)
		ENSURE(!weak_vec[i]);
}

KERNEL_TEST(shared_ptr_refcount) {
	TestClass::num_alloced = 0;
	kstd::SharedPtr<TestClass> first_ptr(new TestClass());
	auto* ref_count = first_ptr.ref_count();
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 1);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::WeakPtr<TestClass> second_ptr = first_ptr;
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 2);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::WeakPtr<TestClass> third_ptr = first_ptr->self();
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 3);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::SharedPtr<TestClass> fourth_ptr = third_ptr.lock();
	ENSURE_EQ(ref_count->strong_count(), 2);
	ENSURE_EQ(ref_count->weak_count(), 3);
	ENSURE_EQ(TestClass::num_alloced, 1);

	first_ptr.reset();
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 3);
	ENSURE_EQ(TestClass::num_alloced, 1);

	fourth_ptr.reset();
	ENSURE_EQ(ref_count->strong_count(), 0);
	ENSURE_EQ(ref_count->weak_count(), 2);
	ENSURE_EQ(TestClass::num_alloced, 0);
}
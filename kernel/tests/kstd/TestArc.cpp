/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "../KernelTest.h"
#include "../../kstd/Arc.h"

using kstd::ArcSelf;
using kstd::Arc;
using kstd::Weak;

class TestClass: public ArcSelf<TestClass> {
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

KERNEL_TEST(basic_arc) {
	TestClass::num_alloced = 0;
	kstd::vector<Arc<TestClass>> vec;
	for(int i = 0; i < 1000; i++)
		vec.push_back(Arc<TestClass>::make());
	ENSURE_EQ(TestClass::num_alloced, 1000);
	for(int i = 0; i < 1000; i++)
		vec.erase(0);
	ENSURE_EQ(TestClass::num_alloced, 0);
}

KERNEL_TEST(basic_weak) {
	TestClass::num_alloced = 0;
	kstd::vector<Arc<TestClass>> vec;
	kstd::vector<Weak<TestClass>> weak_vec;

	for(int i = 0; i < 1000; i++) {
		auto test = Arc<TestClass>::make();
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

KERNEL_TEST(arc_refcount) {
	TestClass::num_alloced = 0;
	kstd::Arc<TestClass> first_ptr(new TestClass());
	auto* ref_count = first_ptr.ref_count();
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 1);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::Weak<TestClass> second_ptr = first_ptr;
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 2);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::Weak<TestClass> third_ptr = first_ptr->self();
	ENSURE_EQ(ref_count->strong_count(), 1);
	ENSURE_EQ(ref_count->weak_count(), 3);
	ENSURE_EQ(TestClass::num_alloced, 1);

	kstd::Arc<TestClass> fourth_ptr = third_ptr.lock();
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

KERNEL_TEST(arc_assignment) {
	TestClass::num_alloced = 0;
	auto arc = Arc<TestClass>::make();
	ENSURE_EQ(TestClass::num_alloced, 1);
	ENSURE_EQ(arc.ref_count()->strong_count(), 1);

	arc = Arc<TestClass>::make();
	ENSURE_EQ(TestClass::num_alloced, 1);
	ENSURE_EQ(arc.ref_count()->strong_count(), 1);

	arc = arc;
	ENSURE_EQ(TestClass::num_alloced, 1);
	ENSURE_EQ(arc.ref_count()->strong_count(), 1);

	auto arc2 = arc;
	ENSURE_EQ(TestClass::num_alloced, 1);
	ENSURE_EQ(arc2.ref_count(), arc.ref_count());
	ENSURE_EQ(arc.ref_count()->strong_count(), 2);

	arc.reset();
	ENSURE_EQ(TestClass::num_alloced, 1);
	ENSURE_EQ(arc2.ref_count()->strong_count(), 1);

	arc2.reset();
	ENSURE_EQ(TestClass::num_alloced, 0);
}
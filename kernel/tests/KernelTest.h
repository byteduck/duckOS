/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <kernel/kstd/vector.hpp>
#include <kernel/kstd/KLog.h>

#define KERNEL_TEST(name) \
	void __test_##name(); \
	static bool __didRegister_test##name = KernelTestRegistry::inst().register_test({#name, __test_##name}); \
	void __test_##name()

#define ENSURE(...) KernelTestRegistry::inst().ensure(__FILE__, __LINE__, __VA_ARGS__)
#define ENSURE_EQ(...) KernelTestRegistry::inst().ensure_eq(__FILE__, __LINE__, __VA_ARGS__)

typedef void (*TestFunc)();
struct KernelTest {
	const char* name;
	TestFunc func;
} __attribute__((aligned(8)));

class KernelTestRegistry {
public:
	static KernelTestRegistry& inst();
	bool register_test(const KernelTest& test);

	void run_tests();

	inline void ensure(const char* file_name, int line_no, bool assertion) {
		if(!assertion) {
			m_passing = false;
			KLog::err(m_current_test->name, "Ensure failed on line {} in {}!", line_no, file_name);
		}
	}

	inline void ensure(const char* file_name, int line_no, bool assertion, const char* message) {
		if(!assertion) {
			m_passing = false;
			KLog::err(m_current_test->name, "Ensure failed on line {} in {}: {}", line_no, file_name, message);
		}
	}

	template<typename A, typename B>
	inline void ensure_eq(const char* file_name, int line_no, const A& param_a, const B& param_b) {
		if(param_a != param_b) {
			m_passing = false;
			// TODO: This is really only useful with ints
			KLog::err(m_current_test->name, "Ensure failed on line {} in {}: {} != {}", line_no, file_name, param_a,
					   param_b);
		}
	}

private:
	static KernelTestRegistry* m_instance;
	kstd::vector<KernelTest> m_tests;
	KernelTest* m_current_test = nullptr;
	bool m_passing = true;
};

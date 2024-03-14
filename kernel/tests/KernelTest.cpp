/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "KernelTest.h"

KernelTestRegistry* KernelTestRegistry::m_instance = nullptr;

KernelTestRegistry& KernelTestRegistry::inst() {
	if(!m_instance) {
		m_instance = new KernelTestRegistry();
	}
	return *m_instance;
}

bool KernelTestRegistry::register_test(const KernelTest& test) {
	m_tests.push_back(test);
	return true;
}

void KernelTestRegistry::run_tests() {
	KLog::info("KernelTests", "Running kernel tests...");
	int n_pass = 0;
	for(size_t i = 0; i < m_tests.size(); i++) {
		m_passing = true;
		auto& test = m_tests[i];
		m_current_test = &test;
		KLog::info("KernelTests", "({}/{}) Testing {}...", i + 1, m_tests.size(), test.name);
		m_tests[i].func();
		if(m_passing) {
			KLog::success("KernelTests", "Test {} passed!", test.name);
			n_pass++;
		} else {
			KLog::err("KernelTests", "Test {} failed!", test.name);
		}
	}

	if(n_pass == m_tests.size()) {
		KLog::success("KernelTests", "Finished tests! (All {} passed)", m_tests.size());
	} else {
		KLog::err("KernelTests", "Finished tests with errors: ({}/{} passed)", n_pass, m_tests.size());
	}
}
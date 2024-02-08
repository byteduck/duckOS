/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "RefCount.h"
#include "../../tasking/Mutex.h"

using namespace kstd;

RefCount::RefCount(int strong_count):
		m_strong_count(strong_count),
		m_weak_count(0) {}

RefCount::RefCount(RefCount&& other):
	m_strong_count(other.m_strong_count),
	m_weak_count(other.m_weak_count) {}

RefCount::~RefCount() {
	ASSERT(m_strong_count.load() == 0);
	ASSERT(m_weak_count.load() == 0);
}

long RefCount::strong_count() const {
	return m_strong_count.load();
}

long RefCount::weak_count() const {
	return m_weak_count.load();
}

bool RefCount::make_strong() {
	long expected = m_strong_count.load();
	if(!expected)
		return false;
	while(!m_strong_count.compare_exchange_strong(expected, expected + 1)) {
		if(!expected)
			return false;
	}
	return true;
}

void RefCount::acquire_strong() {
	auto prev = m_strong_count.add(1);
	ASSERT(prev != 0);
}

void RefCount::acquire_weak() {
	m_weak_count.add(1);
}

PtrReleaseAction RefCount::release_strong() {
	auto prev = m_strong_count.sub(1);
	ASSERT(prev > 0);
	if(prev == 1) {
		if(m_weak_count.load() == 0)
			delete this;
		return PtrReleaseAction::Destroy;
	}
	return PtrReleaseAction::Keep;
}

void RefCount::release_weak() {
	auto prev = m_weak_count.sub(1);
	ASSERT(prev > 0);
	if(prev == 1) {
		if(m_strong_count.load() == 0)
			delete this;
	}
}
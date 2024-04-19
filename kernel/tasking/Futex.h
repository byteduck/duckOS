/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "Blocker.h"
#include "../memory/VMRegion.h"
#include "../filesystem/File.h"

class Futex: public Blocker, public File {
public:
	Futex(kstd::Arc<VMObject> object, size_t offset_in_object);

	// Blocker
	bool is_ready() override;

	// File
	bool can_read(const FileDescriptor& fd) override;

private:
	kstd::Arc<VMObject> m_object;
	kstd::Arc<VMRegion> m_k_region;
	Atomic<int>* m_var;
};

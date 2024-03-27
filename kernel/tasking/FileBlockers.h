/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "Blocker.h"
#include "../filesystem/FileDescriptor.h"

class WriteBlocker: public Blocker {
public:
	WriteBlocker(FileDescriptor& desc);
	bool is_ready() override;

private:
	FileDescriptor& m_desc;
};

class ReadBlocker: public Blocker {
public:
	ReadBlocker(FileDescriptor& desc);
	bool is_ready() override;

private:
	FileDescriptor& m_desc;
};
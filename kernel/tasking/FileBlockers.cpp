/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "FileBlockers.h"

WriteBlocker::WriteBlocker(FileDescriptor& desc): m_desc(desc) {}
bool WriteBlocker::is_ready() {
	return m_desc.file()->can_write(m_desc);
}

ReadBlocker::ReadBlocker(FileDescriptor& desc): m_desc(desc) {}
bool ReadBlocker::is_ready() {
	return m_desc.file()->can_read(m_desc);
}
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "VMRegion.h"
#include "Bytes.h"

class KBuffer: public WriteableBytes {
public:
	static ResultRet<kstd::Arc<KBuffer>> alloc(size_t size);

private:
	explicit KBuffer(const kstd::Arc<VMRegion>& region);

	kstd::Arc<VMRegion> m_region;
};

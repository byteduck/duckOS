/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "VMRegion.h"

VMRegion::VMRegion(kstd::shared_ptr<VMObject> object, size_t start, size_t size):
	m_object(object),
	m_start(start),
	m_size(size)
{

}

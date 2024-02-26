/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#include "TextStorage.h"

using namespace UI;

BasicTextStorage::BasicTextStorage(const char* str): m_str(str) {}

std::string_view BasicTextStorage::text() {
	return m_str;
}

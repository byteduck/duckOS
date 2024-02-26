/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include <string>

namespace UI {
	class ImmutableTextStorage {
	public:
		virtual std::string_view text() = 0;
	};

	class MutableTextStorage {
	public:
		virtual void set_text(std::string_view text) = 0;
	};

	class TextStorage: public ImmutableTextStorage, public MutableTextStorage {};

	class BasicTextStorage: public ImmutableTextStorage {
	public:
		BasicTextStorage(const char* str);
		std::string_view text() override;

	private:
		const char* m_str;
	};
}
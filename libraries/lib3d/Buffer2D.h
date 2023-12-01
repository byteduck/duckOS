/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libgraphics/Geometry.h>

namespace Lib3D {
	template<typename T>
	struct Buffer2D {
	public:
		Buffer2D(size_t width, size_t height):
			m_width(width), m_height(height),
			m_data(new T[width * height]) {}

		Buffer2D(const Buffer2D& other):
			m_width(other.m_width), m_height(other.m_height),
			m_data(new T[other.width, other.height])
		{
			memcpy(m_data, other.m_data, m_width * m_height * sizeof(T));
		}

		Buffer2D(Buffer2D&& other):
			m_width(other.m_width), m_height(other.m_height),
			m_data(other.m_data)
		{
			other.m_data = nullptr;
		}

		Buffer2D& operator=(const Buffer2D& other) {
			delete m_data;
			m_width = other.m_width;
			m_height = other.m_height;
			m_data = new T[m_width * m_height];
			memcpy(m_data, other.m_data, m_width * m_height * sizeof(T));
			return *this;
		}

		Buffer2D& operator=(Buffer2D&& other) noexcept {
			delete m_data;
			m_width = other.m_width;
			m_height = other.m_height;
			m_data = other.m_data;
			other.m_data = nullptr;
			return *this;
		}

		~Buffer2D() {
			delete m_data;
		}

		inline T& at(size_t x, size_t y) {
			return m_data[x + y * m_width];
		}

		inline T at(size_t x, size_t y) const {
			return m_data[x + y * m_width];
		}

		inline void set(size_t x, size_t y, T val) {
			if(x < m_width && y < m_height)
				at(x, y) = val;
		}

		inline T get(size_t x, size_t y) const {
			if(x < m_width && y < m_height)
				return at(x, y);
			return {};
		}

		void fill(T val) {
			for(int i = 0; i < m_width * m_height; i++)
				m_data[i] = val;
		}

		size_t width() const { return m_width; }
		size_t height() const { return m_height; }
		T* data() const { return m_data; }

	private:
		size_t m_width, m_height;
		T* m_data;
	};
}
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <stddef.h>
#include <math.h>
#include <initializer_list>

template<typename T, size_t N>
struct Vec {
public:
	constexpr Vec(): m_storage{{0}} {}
	constexpr Vec(std::initializer_list<T> elements) {
		int i = 0;
		for(auto& element : elements)
			m_storage[i++] = element;
	}
	constexpr Vec(T x, T y): m_storage{x, y} {}
	constexpr Vec(T x, T y, T z): m_storage{x, y, z} {}
	constexpr Vec(T x, T y, T z, T w): m_storage{x, y, z, w} {}

	[[nodiscard]] constexpr T x() const { return m_storage[0]; }
	[[nodiscard]] constexpr T& x() { return m_storage[0]; }
	[[nodiscard]] constexpr T y() const { return m_storage[1]; }
	[[nodiscard]] constexpr T& y() { return m_storage[1]; }
	[[nodiscard]] constexpr T z() const { return m_storage[2]; }
	[[nodiscard]] constexpr T& z() { return m_storage[2]; }
	[[nodiscard]] constexpr T w() const { return m_storage[3]; }
	[[nodiscard]] constexpr T& w() { return m_storage[3]; }

	constexpr const T& operator[](int idx) const {
		return m_storage[idx];
	};

	constexpr T& operator[](int idx) {
		return m_storage[idx];
	};

	#define __VEC_OP_DEF(op) \
		Vec<T,N> constexpr operator op (const Vec<T,N> other) const { \
			Vec<T,N> out; \
			for(int i = 0; i < N; i++) \
				out.m_storage[i] = m_storage[i] op other.m_storage[i]; \
			return out; \
		}

	#define __VEC_SELFOP_DEF(op) \
		Vec<T,N> constexpr operator op (const Vec<T,N> other) { \
			for(int i = 0; i < N; i++) \
				m_storage[i] op other.m_storage[i]; \
			return *this; \
		}

	#define __VEC_SCALAR_OP_DEF(op) \
		Vec<T,N> constexpr operator op (const T scalar) const { \
			Vec<T,N> out; \
			for(int i = 0; i < N; i++) \
				out.m_storage[i] = m_storage[i] op scalar; \
			return out; \
		}

	#define __VEC_SCALAR_SELFOP_DEF(op) \
		Vec<T,N> constexpr operator op (const T scalar) { \
			for(int i = 0; i < N; i++) \
				m_storage[i] op scalar; \
			return *this; \
		}

	__VEC_OP_DEF(+);
	__VEC_SELFOP_DEF(+=);
	__VEC_OP_DEF(-);
	__VEC_SELFOP_DEF(-=);

	__VEC_SCALAR_OP_DEF(+);
	__VEC_SCALAR_SELFOP_DEF(+=);
	__VEC_SCALAR_OP_DEF(-);
	__VEC_SCALAR_SELFOP_DEF(-=);
	__VEC_SCALAR_OP_DEF(*);
	__VEC_SCALAR_SELFOP_DEF(*=);
	__VEC_SCALAR_OP_DEF(/);
	__VEC_SCALAR_SELFOP_DEF(/=);

	T constexpr operator*(const Vec<T, N> other) const {
		T dot = {};
		for(int i = 0; i < N; i++)
			dot += m_storage[i] * other.m_storage[i];
		return dot;
	}

	Vec<T, 3> constexpr operator^(const Vec<T, 3> other) const {
		return {
			y() * other.z() - z() * other.y(),
			z() * other.x() - x() * other.z(),
			x() * other.y() - y() * other.x()
		};
	}

	T constexpr magnitude() {
		T sqr_total = {};
		for(int i = 0; i < N; i++)
			sqr_total += m_storage[i] * m_storage[i];
		return sqrt(sqr_total);
	}

	Vec<T, N> constexpr normalize() {
		return *this / magnitude();
	}

private:
	T m_storage[N];
};

using Vec2f = Vec<float, 2>;
using Vec3f = Vec<float, 3>;
using Vec4f = Vec<float, 4>;
using Vec2i = Vec<int, 2>;
using Vec3i = Vec<int, 3>;
using Vec4i = Vec<int, 4>;
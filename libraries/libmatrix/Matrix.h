/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include "Vec.h"
#include <assert.h>

template<typename T, size_t M, size_t N = 1>
struct Matrix {
public:
	Matrix() = default;

	constexpr Matrix(std::initializer_list<Vec<T, N>> rows) {
		assert(rows.size() == M);
		int i = 0;
		for(auto& row : rows)
			m_rows[i++] = row;
	}

	constexpr Vec<T, N>& operator[](int idx) {
		return m_rows[idx];
	};

	Matrix<T, N, M> constexpr transpose() const {
		Matrix<T, N, M> ret;
		for(int i = 0; i < N; i++) {
			auto& row = ret.m_rows[i];
			for(int j = 0; j < M; j++) {
				row[j] = m_rows[j][i];
			}
		}
		return ret;
	}

	Vec<T, M> constexpr col(int idx) const {
		Vec<T, M> ret;
		for(int i = 0; i < M; i++)
			ret[i] = m_rows[i][idx];
		return ret;
	}

	template<size_t P>
	Matrix<T, M, P> constexpr operator*(Matrix<T, N, P> const& other) const {
		Matrix<T, M, P> ret;
		for(int i = 0; i < M; i++) {
			auto& row = ret.m_rows[i];
			for(int j = 0; j < P; j++) {
				// Special cases for certain matrices to optimize
				if constexpr(N == 1) {
					row[j] = m_rows[i].x() * other.m_rows[0][j];
				} else if constexpr(N == 2) {
					row[j] = m_rows[i].x() * other.m_rows[0][j] +
							 m_rows[i].y() * other.m_rows[1][j];
				} else if constexpr(N == 3) {
					row[j] = m_rows[i].x() * other.m_rows[0][j] +
							 m_rows[i].y() * other.m_rows[1][j] +
							 m_rows[i].z() * other.m_rows[2][j];
				} else if constexpr(N == 4) {
					row[j] = m_rows[i].x() * other.m_rows[0][j] +
							m_rows[i].y() * other.m_rows[1][j] +
							m_rows[i].z() * other.m_rows[2][j] +
							m_rows[i].w() * other.m_rows[3][j];
				} else {
					for(int k = 0; k < N; k++)
						row[j] += m_rows[i][k] * other.m_rows[k][j];
				}
			}
		}
		return ret;
	}

	Matrix<T, M, 1> constexpr operator*(Vec<T, M> const& vec) const {
		Matrix<T, M, 1> ret;
		for(int i = 0; i < M; i++) {
			auto& row = ret.m_rows[i][0];
			for(int k = 0; k < N; k++)
				row += m_rows[i][k] * vec[k];
		}
		return ret;
	}

	Matrix<T, M, N> constexpr operator+(Matrix<T, M, N> const& other) const {
		Matrix<T, M, N> ret;
		for(int i = 0; i < M; i++)
			ret[i] = m_rows[i] + other.m_rows[i];
		return ret;
	}

	Matrix<T, M, N> constexpr operator-(Matrix<T, M, N> const& other) const {
		Matrix<T, M, N> ret;
		for(int i = 0; i < M; i++)
			ret[i] = m_rows[i] + other.m_rows[i];
		return ret;
	}

	Matrix<T, M, N> constexpr operator*(T scalar) const {
		Matrix<T, M, N> ret;
		for(int i = 0; i < M; i++)
			ret[i] = m_rows[i] * scalar;
		return ret;
	}

	Vec<T, N> m_rows[M];
};

using Matrix3f = Matrix<float, 3, 3>;
using Matrix4f = Matrix<float, 4, 4>;
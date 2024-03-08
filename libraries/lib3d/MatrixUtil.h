/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2023 Byteduck */

#pragma once

#include <libmatrix/Matrix.h>
#include <libgraphics/Color.h>
#include <math.h>
#include <libgraphics/Framebuffer.h>

namespace Lib3D {
	Matrix4f constexpr ortho(float left, float right, float bottom, float top, float near, float far) {
		return {{2.0f / (right - left), 0,              0,             -(right+left)/(right-left)},
				{0,                     2/(top-bottom), 0,             -(top+bottom)/(top-bottom)},
				{0,                     0,              -2/(far-near), -(far+near)/(far-near)    },
				{0,                     0,              0,             1                         }};
	}

	template<typename T, size_t N>
	Matrix<T, N, N> constexpr identity() {
		Matrix<T, N, N> ret;
		for (int i = 0; i < N; i++)
			ret[i][i] = 1;
		return ret;
	}

	Matrix4f constexpr rotate(float angle, Vec3f axis) {
		const float cosx = cosf(axis.x() * angle);
		const float sinx = sinf(axis.x() * angle);
		const float cosy = cosf(axis.y() * angle);
		const float siny = sinf(axis.y() * angle);
		const float cosz = cosf(axis.z() * angle);
		const float sinz = sinf(axis.z() * angle);
		return {{cosx * cosy, cosx * siny * sinz - sinx * cosz, cosx * siny * cosz + sinx * sinz, 0},
				{sinx * cosy, sinx * siny * sinz + cosx * cosz, sinx * siny * cosz - cosx * sinz, 0},
				{-siny, cosy * sinz, cosy * cosz, 0},
				{0,0,0,1}};
	}

	Matrix4f constexpr scale(float x, float y, float z) {
		return {
			{x, 0, 0, 0},
			{0, y, 0, 0},
			{0, 0, z, 0},
			{0, 0, 0, 1}
		};
	}

	Matrix4f constexpr scale(Vec3f trans) {
		return scale(trans.x(), trans.y(), trans.z());
	}

	Matrix4f constexpr translate(float x, float y, float z) {
		return {
				{1, 0, 0, x},
				{0, 1, 0, y},
				{0, 0, 1, z},
				{0, 0, 0, 1}
		};
	}

	Matrix4f constexpr translate(Vec3f trans) {
		return translate(trans.x(), trans.y(), trans.z());
	}

	float constexpr det3f(Matrix3f mat) {
		return mat[0][0] * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) -
		       mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]) +
			   mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
	}

	Gfx::Color constexpr vec_to_color(Vec4f vec) {
		return { (uint8_t) (vec.x() * 255),
				 (uint8_t) (vec.y() * 255),
				 (uint8_t) (vec.z() * 255),
				 (uint8_t) (vec.w() * 255) };
	}

	Vec4f constexpr color_to_vec(Gfx::Color color) {
		return { (float) color.r / 255.0f,
				 (float) color.g / 255.0f,
				 (float) color.b / 255.0f,
				 (float) color.a / 255.0f };
	}
}
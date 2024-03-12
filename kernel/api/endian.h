/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2024 Byteduck */

#pragma once

#include "stdint.h"

#ifdef __cplusplus

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr static bool __little_endian = true;
#else
constexpr static bool __little_endian = false;
#endif

template<typename T>
inline constexpr T swap_endianness(T val) {
	if constexpr(sizeof(T) == 1)
		return val;
	else if constexpr (sizeof(T) == 2)
		return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(val)));
	else if constexpr (sizeof(T) == 4)
		return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(val)));
	else if constexpr (sizeof(T) == 8)
		return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(val)));
	else
		static_assert("Cannot swap endianness of anything larger than 8 bytes");
}

template<typename T>
inline constexpr T as_little_endian(T val) {
	if constexpr(__little_endian)
		return val;
	else
		return swap_endianness(val);
}

template<typename T>
inline constexpr T as_big_endian(T val) {
	if constexpr(!__little_endian)
		return val;
	else
		return swap_endianness(val);
}

template<typename T>
inline constexpr T from_little_endian(T val) {
	if constexpr(__little_endian)
		return val;
	else
		return swap_endianness(val);
}

template<typename T>
inline constexpr T from_big_endian(T val) {
	if constexpr(!__little_endian)
		return val;
	else
		return swap_endianness(val);
}

template<typename T>
class LittleEndian {
public:
	constexpr LittleEndian() = default;
	constexpr LittleEndian(T val): m_val(as_little_endian(val)) {}
	constexpr operator T() const { return from_little_endian(m_val); }
	constexpr T val() const { return operator T(); }
private:
	T m_val;
} __attribute__((packed));

template<typename T>
class BigEndian {
public:
	constexpr BigEndian() = default;
	constexpr BigEndian(T val): m_val(as_big_endian(val)) {}
	constexpr operator T() const { return from_big_endian(m_val); }
	constexpr T val() const { return operator T(); }
private:
	T m_val;
} __attribute__((packed));

#endif
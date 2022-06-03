/*
    This file is part of duckOS.
    
    duckOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    duckOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with duckOS.  If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) Byteduck 2016-2022. All rights reserved.
*/

#pragma once

#include <cstddef>
#include "serialization_utils.h"

namespace Duck {
	class Serializable {
	public:
		virtual ~Serializable() = default;

		/**
		 * Gets the serialized size of the object.
		 * @return The serialized size, in bytes, of the object.
		 */
		virtual size_t serialized_size() const = 0;

		/**
		 * Serializes the object.
		 * @param buf The buffer to serialize into.
		 * @return The new buffer pointer (ie a pointer to right after the serialized object)
		 */
		virtual uint8_t* serialize(uint8_t* buf) const = 0;

		/**
		 * Deserializes the object.
		 * @param buf The buffer to deserialize from.
		 * @return The new buffer pointer (ie a pointer to right after the serialized object)
		 */
		virtual const uint8_t* deserialize(const uint8_t* buf) = 0;
	};
}
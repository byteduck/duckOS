/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "../kstd/Bitmap.h"
#include "PhysicalPage.h"
#include "Memory.h"
#include "../Result.hpp"


/**
 * Inspired by Linux's buddy system for memory allocation.
 * https://www.kernel.org/doc/gorman/html/understand/understand009.html
 */

enum class BuddyState {
	BOTH = false, // Both buddies are free or allocated.
	MIXED = true // One buddy is free, the other is allocated.
};

/**
 * A struct representing a bucket of blocks in a certain order of a buddy zone.
 */
struct BuddyOrder {
	/** Gets the index of the buddy bit for the given block index. */
	inline size_t bit_index(int block_index) const {
		return (size_t) (block_index >> order) / 2;
	}

	/** Gets the buddy bit for the given block index. */
	inline BuddyState get_bit(int block_index) const {
		return (BuddyState) block_map.get(bit_index(block_index));
	}

	/** Sets the buddy bit for the given block index. */
	inline void set_bit(int block_index, BuddyState val) {
		block_map.set(bit_index(block_index), (bool) val);
	}

	unsigned int order; ///< The order of the block in the buddy system.
	kstd::Bitmap block_map; ///< The map of blocks in the order bucket.
	int freelist = -1; ///< The index to the first page in the freelist relative to the zone's first page. -1 == None free.
};

class BuddyZone {
public:
	BuddyZone(size_t first_page, size_t num_pages);

	constexpr static int MAX_ORDER = 10;
	constexpr static size_t MAX_ZONE_SIZE = 1 << MAX_ORDER;

	/**
	 * Calculates the order needed for a given block size (in pages).
	 * @param num_pages The number of pages needed.
	 */
	inline static unsigned order_for(unsigned int num_pages) {
		// Essentially, calculate the index of the highest bit, and add another order if the number is more than 2^n
		unsigned int highest_bit = (sizeof(unsigned int) * 8) - __builtin_clz(num_pages) - 1;
		return (num_pages - (1 << highest_bit)) ? highest_bit + 1 : highest_bit;
	}

	/** Calculates the size, in pages, of one block in an order. */
	constexpr static unsigned int size_of_order(unsigned int order) {
		return 1 << order;
	}

	/** Calculates the page index of the given block index within this zone. **/
	inline unsigned int page_of_block(int index) const {
		return m_first_page + index;
	}

	/** Gets the page corresponding to a block. */
	PhysicalPage& get_block(int block) const;

	/**
	 * Allocates a block in this zone.
	 * @param num_pages The size of the block requested, in pages.
	 * @returns A result containing the index of the first page allocated.
	 */
	ResultRet<PageIndex> alloc_block(size_t num_pages);

	/**
	 * Frees a block in this zone.
	 * @param start_page The start page of the block to free.
	 * @param num_pages The size of the block to free, in pages.
	 * @returns A result containing the index of the first page allocated.
	 */
	void free_block(PageIndex start_page, size_t num_pages);

	PageIndex first_page() const { return m_first_page; }
	size_t num_pages() const { return m_num_pages; }
	bool contains_page(PageIndex page) const {
		return page >= m_first_page && page < m_first_page + m_num_pages;
	}

private:
	/**
	 * Allocates a block in this zone.
	 * @param order The order of the block requested.
	 * @returns A result containing the index of the first page allocated relative to this zone.
	 */
	ResultRet<int> alloc_block_internal(unsigned int order);

	/**
	 * Frees a block in this zone.
	 * @param block The index of the block to free, relative to this zone.
	 * @param order The order of the block to free.
	 */
	void free_block_internal(int block, unsigned int order);

	/** Removes the given block from the freelist of the given order */
	void remove_from_freelist(int block, unsigned int order);

	BuddyOrder m_orders[MAX_ORDER + 1]; ///< The orders in the zone, with order 0 being 1 page blocks.
	PageIndex m_first_page; ///< The index of the first page of this zone.
	unsigned int m_highest_order; ///< The highest order of this zone.
	size_t m_num_pages; ///< The number of pages in this zone.
	size_t m_free_pages; ///< The number of free pages in this zone.
};

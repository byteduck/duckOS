/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#include "BuddyZone.h"
#include "MemoryManager.h"

BuddyZone::BuddyZone(PageIndex first_page, size_t num_pages):
	m_first_page(first_page),
	m_highest_order(order_for(num_pages)),
	m_num_pages(num_pages),
	m_free_pages(num_pages)
{
	ASSERT(size_of_order(m_highest_order) == num_pages); // Make sure the number of pages is a power of 2
	ASSERT(m_highest_order <= MAX_ORDER); // Make sure the highest order is less than or equal to the max order

	// Initialize the bitmaps for each order
	for(int i = 0; i <= m_highest_order; i++) {
		auto& bucket = m_orders[i];
		bucket.order = i;
		auto num_blocks = num_pages / size_of_order(i);
		auto order_bitmap_size = (num_blocks + 1) / 2;
		if(order_bitmap_size)
			bucket.block_map = kstd::Bitmap(order_bitmap_size);
	}

	m_orders[m_highest_order].freelist = 0;
	m_orders[m_highest_order].set_bit(0, BuddyState::MIXED);
}

PhysicalPage& BuddyZone::get_block(int block) const {
	return MemoryManager::inst().get_physical_page(page_of_block(block));
}

ResultRet<PageIndex> BuddyZone::alloc_block(size_t num_pages) {
	if(m_free_pages < num_pages)
		return Result(ENOMEM);

	auto block_index = TRY(alloc_block_internal(order_for(num_pages)));
	m_free_pages -= num_pages;
	return m_first_page + block_index;
}

void BuddyZone::free_block(PageIndex start_page, size_t num_pages) {
	ASSERT(start_page >= m_first_page);
	ASSERT(start_page + num_pages <= m_first_page + m_num_pages);
	ASSERT(size_of_order(order_for(num_pages)) == num_pages);
	free_block_internal(start_page - m_first_page, order_for(num_pages));
	m_free_pages += num_pages;
}

ResultRet<int> BuddyZone::alloc_block_internal(unsigned int order) {
	ASSERT(order <= m_highest_order);

	auto& bucket = m_orders[order];
	if(bucket.freelist == -1) {
		// If the freelist is empty, split a higher-order block
		auto higher_order_block = TRY(alloc_block_internal(order + 1));

		// Mark the first block used, second block free
		bucket.set_bit(higher_order_block, BuddyState::MIXED);

		// Update freelist to point to second block
		bucket.freelist = higher_order_block + size_of_order(order);
		auto& free_page = get_block(bucket.freelist);
		free_page.free.next = -1;
		free_page.free.prev = -1;

		return higher_order_block;
	}

	// Otherwise, get the first block in the freelist
	auto block = bucket.freelist;

	// The state should always be mixed, because the first block in the buddy set would have been given to a lower order
	ASSERT(bucket.get_bit(block) == BuddyState::MIXED);

	// Update the freelist and bitmap
	bucket.set_bit(block, BuddyState::BOTH);
	bucket.freelist = get_block(bucket.freelist).free.next;
	get_block(bucket.freelist).free.prev = -1;

	return block;
}

void BuddyZone::free_block_internal(int block, unsigned int order) {
	ASSERT(order <= m_highest_order);

	auto& bucket = m_orders[order];
	if(bucket.get_bit(block) == BuddyState::MIXED) {
		// The buddy was already free, so now merge them and give them to the higher-order block
		bucket.set_bit(block, BuddyState::BOTH);

		// If the block we're freeing is the first buddy block, remove the second from the freelist and vice versa
		int first_buddy_index = bucket.bit_index(block) * 2 * size_of_order(order);
		if(block == first_buddy_index)
			remove_from_freelist(first_buddy_index + size_of_order(order), order);
		else
			remove_from_freelist(first_buddy_index, order);

		// Merge them and hand them to the higher-order bucket
		free_block_internal(first_buddy_index, order + 1);
	} else {
		// The buddy isn't free, so set the state to mixed and add to the beginning of the freelist
		bucket.set_bit(block, BuddyState::MIXED);

		if(bucket.freelist != -1)
			get_block(bucket.freelist).free.prev = (int16_t) block;

		auto& page = get_block(block);
		page.free.next = (int16_t) bucket.freelist;
		page.free.prev = -1;
		bucket.freelist = block;
	}
}

void BuddyZone::remove_from_freelist(int block, unsigned int order) {
	auto& page = get_block(block).free;
	if(page.next != -1)
		get_block(page.next).free.prev = page.prev;
	if(page.prev != -1)
		get_block(page.prev).free.next = page.next;
	if(m_orders[order].freelist == block) {
		ASSERT(page.prev == -1);
		m_orders[order].freelist = page.next;
	}
}

/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include "map.hpp"
#include "vector.hpp"
#include "pair.hpp"
#include "../Result.hpp"
#include "Optional.h"

namespace kstd {
	template<typename Key, typename Value>
	class LRUCache {
	public:
		LRUCache() = default;

		/** Insert the item with the given key and value, replacing it if it exists. **/
		void insert(Key key, Value value) {
			auto node = m_map.get(key);
			if(node) {
				*node = value;
				promote(key);
			} else {
				m_map.insert({key, value});
				m_lru_list.push_back(key);
			}
		}

		/** Removes the item with the given key if it exists. **/
		void erase(Key key) {
			m_map.erase(key);
			for(size_t i = 0; i < m_lru_list.size(); i++) {
				if(m_lru_list[i] == key) {
					m_lru_list.erase(i);
					return;
				}
			}
		}

		/** Promote the item with the given key, if in the list, to be most recently used. **/
		void promote(Key key) {
			bool found = false;
			for(size_t i = 0; i < m_lru_list.size(); i++) {
				if(m_lru_list[i] == key) {
					m_lru_list.erase(i);
					found = true;
					break;
				}
			}

			if(found)
				m_lru_list.push_back(key);
		}

		/** Gets the item with the given key **/
		kstd::Optional<Value> get(Key key) {
			auto item = m_map.get(key);
			if(item) {
				promote(key);
				return *item;
			}
			return kstd::nullopt;
		}

		/** Prunes a number of items from the cache. **/
		void prune(size_t num) {
			while(!m_lru_list.empty() && num--) {
				m_map.erase(m_lru_list[0]);
				m_lru_list.erase(0);
			}
		}

		/** Returns the least recently used item. **/
		kstd::Optional<kstd::pair<Key, Value&>> lru() {
			if(empty())
				return kstd::nullopt;
			return kstd::pair<Key, Value&> {m_lru_list[0], m_map[m_lru_list[0]]};
		}

		/** Returns the least recently used item without wrapping in an optional. **/
		kstd::pair<Key, Value&> lru_unsafe() {
			ASSERT(!empty());
			return kstd::pair<Key, Value&> {m_lru_list[0], m_map[m_lru_list[0]]};
		}

		[[nodiscard]] size_t size() const { return m_lru_list.size(); }
		[[nodiscard]] bool empty() const { return m_lru_list.empty(); }

	private:
		kstd::map<Key, Value> m_map;
		kstd::vector<Key> m_lru_list;
	};
}
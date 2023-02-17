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

	Copyright (c) Byteduck 2016-2021. All rights reserved.
*/

#pragma once

#include "utility.h"
#include "pair.hpp"
#include "Optional.h"

namespace kstd {
	template<typename MapType>
	class MapIterator;

	template<typename K, typename V>
	class map {
	public:
		using Key = K;
		using Val = V;

		class Node {
		public:
			Node(const pair<Key, Val>& data): data(data) {}

			pair<Key, Val> data;
			Node* left = nullptr;
			Node* right = nullptr;
			Node* parent = nullptr;

			Node* inorder_predecessor() {
				// Predecessor is in left subtree
				if(left)
					return subtree_predecessor();

				// Predecessor is rightmost ancestor
				Node* cur = this;
				while(cur && cur->parent && cur->parent->left == cur)
					cur = cur->parent;
				return cur->parent;
			}
			
			Node* subtree_predecessor() {
				if(!left)
					return nullptr;
				auto* cur_node = left;
				while(cur_node->right)
					cur_node = cur_node->right;
				return cur_node;
			}

			Node* inorder_successor() {
				// Successor is in right subtree
				if(right)
					return subtree_successor();

				// Successor is leftmost ancestor
				Node* cur = this;
				while(cur && cur->parent && cur->parent->right == cur)
					cur = cur->parent;
				return cur->parent;
			}
			
			Node* subtree_successor() {
				if(!right)
					return nullptr;
				auto* cur_node = right;
				while(cur_node->left)
					cur_node = cur_node->left;
				return cur_node;
			}
			
			void delete_children() {
				if(left) {
					left->delete_children();
					delete left;
				}
				if(right) {
					right->delete_children();
					delete right;
				}
			}
		};

		map() = default;
		~map() {
			if(m_root) {
				m_root->delete_children();
				delete m_root;
			}
		}

		Node* find_node(const Key& key) const {
			if(!m_root) {
				return nullptr;
			}

			auto* cur_node = m_root;
			while(cur_node) {
				if(cur_node->data.first == key) {
					return cur_node;
				} else if(key < cur_node->data.first) {
					cur_node = cur_node->left;
				} else {
					cur_node = cur_node->right;
				}
			}

			return nullptr;
		}

		bool contains(const Key& key) const {
			return find_node(key);
		}

		Node* insert(const pair<Key, Val>& elem) {
			// TODO: Balance on insertion
			if(!m_root) {
				m_root = new Node(elem);
				m_size++;
				return m_root;
			}

			auto* cur_node = m_root;
			while(true) {
				if(cur_node->data.first == elem.first) {
					return nullptr;
				} else if(elem.first < cur_node->data.first) {
					if(cur_node->left) {
						cur_node = cur_node->left;
					} else {
						cur_node->left = new Node(elem);
						cur_node->left->parent = cur_node;
						m_size++;
						return cur_node->left;
					}
				} else {
					if(cur_node->right) {
						cur_node = cur_node->right;
					} else {
						cur_node->right = new Node(elem);
						cur_node->right->parent = cur_node;
						m_size++;
						return cur_node->right;
					}
				}
			}
		}

		void erase(const Key& key) {
			// TODO: Balance on erase
			if(!m_root)
				return;

			Node* prev_node = nullptr;
			auto* cur_node = m_root;
			while(cur_node) {
				if(cur_node->data.first == key) {
					m_size--;
					if(!cur_node->left && !cur_node->right) {
						// If the node doesn't have children, just remove it
						if(prev_node && prev_node->left == cur_node) {
							prev_node->left = nullptr;
						} else if(prev_node) {
							prev_node->right = nullptr;
						} else {
							m_root = nullptr;
						}
						delete cur_node;
					} else if(cur_node == m_root || cur_node == prev_node->right) {
						// If we're deleting the root or a node that's the right child, replace it with the successor
						auto successor = cur_node->subtree_successor();
						if(successor) {
							cur_node->data = successor->data;
							if(successor->parent == cur_node) {
								cur_node->right = successor->right;
								if(cur_node->right)
									cur_node->right->parent = cur_node;
							} else {
								successor->parent->left = successor->right;
								if(successor->parent->left)
									successor->parent->left->parent = successor->parent;
							}
							delete successor;
						} else {
							// We don't have a successor - just replace the node with the left child.
							cur_node->data = cur_node->left->data;
							cur_node->right = cur_node->left->right;
							if(cur_node->right)
								cur_node->right->parent = cur_node;
							auto* old_left = cur_node->left;
							cur_node->left = cur_node->left->left;
							if(cur_node->left)
								cur_node->left->parent = cur_node;
							delete old_left;
						}
					} else {
						// If we're deleting the root or a node that's the left child, replace it with the predecessor
						auto predecessor = cur_node->subtree_predecessor();
						if(predecessor) {
							cur_node->data = predecessor->data;
							if(predecessor->parent == cur_node) {
								cur_node->left = predecessor->left;
								if(cur_node->left)
									cur_node->left->parent = cur_node;
							} else {
								predecessor->parent->right = predecessor->left;
								if(predecessor->parent->right)
									predecessor->parent->right->parent = predecessor->parent;
							}
							delete predecessor;
						} else {
							// We don't have a predecessor - just replace the node with the right child.
							cur_node->data = cur_node->right->data;
							cur_node->left = cur_node->right->left;
							if(cur_node->left)
								cur_node->left->parent = cur_node;
							auto* old_right = cur_node->right;
							cur_node->right = cur_node->right->right;
							if(cur_node->right)
								cur_node->right->parent = cur_node;
							delete old_right;
						}
					}
					
					break;
				}
				
				prev_node = cur_node;
				if(key < cur_node->data.first)
					cur_node = cur_node->left;
				else
					cur_node = cur_node->right;
			}
		}

		Val& operator[](const Key& key) {
			auto* ret = find_node(key);
			if(!ret)
				return insert({key, Val()})->data.second;
			return ret->data.second;
		}

		Val* get(const Key& key) {
			auto ret = find_node(key);
			if(!ret)
				return nullptr;
			return &ret->data.second;
		}
		
		size_t size() {
			return m_size;
		}

		bool empty() {
			return m_size == 0;
		}

		using Iterator = MapIterator<map<Key, Val>>;
		using ConstIterator = MapIterator<map<Key, Val>>;

		Iterator begin() { return Iterator::begin(*this); }
		ConstIterator begin() const { return ConstIterator::begin(*this); }
		Iterator end() { return Iterator::end(*this); }
		ConstIterator end() const { return ConstIterator::end(*this); }

	private:
		friend class MapIterator<map<Key, Val>>;
		friend class MapIterator<const map<Key, Val>>;
		Node* m_root = nullptr;
		size_t m_size = 0;
	};

	template<typename MapType>
	class MapIterator {
	public:
		MapIterator(const MapIterator& other) = default;
		MapIterator& operator=(const MapIterator& other) {
			m_node = other.m_node;
			m_map = other.m_map;
			return *this;
		}

		constexpr bool operator==(MapIterator other) const { return m_node == other.m_node; }
		constexpr bool operator!=(MapIterator other) const { return m_node != other.m_node; }
		constexpr bool operator>=(MapIterator other) const { return m_node->data.first >= m_node->data.first; }
		constexpr bool operator<=(MapIterator other) const { return m_node->data.first <= m_node->data.first; }
		constexpr bool operator<(MapIterator other) const { return m_node->data.first < m_node->data.first; }
		constexpr bool operator>(MapIterator other) const { return m_node->data.first > m_node->data.first; }

		constexpr MapIterator operator++() {
			if(m_node)
				m_node = m_node->inorder_successor();
			return *this;
		}

		constexpr MapIterator operator++(int) {
			auto old_node = m_node;
			if(m_node)
				m_node = m_node->inorder_successor();
			return { m_map, old_node };
		}

		constexpr MapIterator operator--() {
			// If m_node == nullptr, we are the end. Thus, find the rightmost element in the tree
			if(!m_node) {
				if(!m_map.m_root)
					return *this;
				m_node = m_map.m_root;
				while(m_node->right)
					m_node = m_node->right;
			} else {
				m_node = m_node->inorder_predecessor();
			}
			return *this;
		}

		constexpr MapIterator operator--(int) {
			auto old_node = m_node;
			operator--();
			return { m_map, old_node };
		}

		using PairType = pair<typename MapType::Key, typename MapType::Val>;
		inline constexpr const PairType* operator->() const { return &m_node->data; }
		inline constexpr PairType* operator->() { return &m_node->data; }
		inline constexpr const PairType& operator*() const { return m_node->data; }
		inline constexpr PairType& operator*() { return m_node->data; }

		MapIterator operator+(ptrdiff_t diff) const {
			MapIterator ret = { m_map, m_node };
			if(diff > 0) {
				while(diff--)
					ret++;
			} else {
				while(diff++)
					ret--;
			}
			return ret;
		}

		MapIterator operator-(ptrdiff_t diff) const { return operator+(-diff); }


	private:
		friend MapType;

		constexpr MapIterator(MapType& m_map, MapType::Node* node):
				m_map(m_map),
				m_node(node) {}

		static constexpr MapIterator begin(MapType& map) {
			if(!map.m_root)
				return {map, nullptr};
			auto node = map.m_root;
			while(node->left)
				node = node->left;
			return {map, node};
		}

		static constexpr MapIterator end(MapType& map) { return { map, nullptr }; }

		MapType& m_map;
		MapType::Node* m_node;
	};
}

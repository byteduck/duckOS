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

	/** An implementation of a map using an AVL tree. **/
	template<typename K, typename V>
	class map {
	public:
		using Key = K;
		using Val = V;

		class Node {
		public:
			Node(const pair<Key, Val>& data): data(data) {}

			using MapType = map<Key, Val>;

			pair<Key, Val> data;
			Node* left = nullptr;
			Node* right = nullptr;
			Node* parent = nullptr;
			int height = 1;

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

			/** Calculates the balance factor of this node. **/
			int balance_factor() const {
				int bf = 0;
				if(left)
					bf -= left->height;
				if(right)
					bf += right->height;
				return bf;
			}

		private:
			friend MapType;

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

			/** Updates a node and its parents' heights. **/
			void update_height_and_balance(MapType* map) {
				auto cur_node = this;
				while(cur_node) {
					cur_node->update_own_height();
					cur_node->balance(map);
					cur_node = cur_node->parent;
				}
			}

			/** Balances this node if needed. **/
			void balance(MapType* map) {
				int bfc = balance_factor();
				if(bfc > -2 && bfc < 2)
					return;
				if(bfc < -1) {
					ASSERT(left);
					// Left-heavy. If left node is right-heavy, we need to rotate that left first
					if(left->balance_factor() > 0)
						left->rotate_left(map);
					rotate_right(map);
				} else {
					// Right-heavy. If right node is left-heavy, we need to rotate that right first
					ASSERT(right);
					if(right->balance_factor() < 0)
						right->rotate_right(map);
					rotate_left(map);
				}
			}

			/** Performs a left AVL rotation on this and the right node. **/
			void rotate_left(MapType* map) {
				// Perform rotation
				auto old_right = right;
				auto old_right_left = right->left;
				right->left = this;
				right = old_right_left;
				old_right->parent = parent;
				if(parent) {
					if(parent->left == this)
						parent->left = old_right;
					else
						parent->right = old_right;
				}
				parent = old_right;
				if(right)
					right->parent = this;

				// Update heights
				update_own_height();
				old_right->update_own_height();

				// Update root of tree if needed
				if(map->m_root == this)
					map->m_root = old_right;
			}

			/** Performs a right AVL rotation on this and the right node. **/
			void rotate_right(MapType* map) {
//				// Perform rotation
				auto old_left = left;
				auto old_left_right = left->right;
				left->right = this;
				left = old_left_right;
				old_left->parent = parent;
				if(parent) {
					if(parent->left == this)
						parent->left = old_left;
					else
						parent->right = old_left;
				}
				parent = old_left;
				if(left)
					left->parent = this;

				// Update heights
				update_own_height();
				old_left->update_own_height();

				// Update root of tree if needed
				if(map->m_root == this)
					map->m_root = old_left;
			}

			inline void update_own_height() {
				height = max(right ? right->height : 0, left ? left->height : 0) + 1;
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
			if(!m_root) {
				m_root = new Node(elem);
				m_size++;
				return m_root;
			}

			auto insert_at = [&] (Node* node, Node*& slot) -> Node* {
				auto new_node = new Node(elem);
				slot = new_node;
				slot->parent = node;
				m_size++;
				if(node->height == 1)
					node->update_height_and_balance(this); // slot may be invalid after rebalancing
				return new_node;
			};

			auto* cur_node = m_root;
			while(true) {
				if(cur_node->data.first == elem.first) {
					// We already have this element
					return nullptr;
				} else if(elem.first < cur_node->data.first) {
					// Go left or insert left
					if(cur_node->left)
						cur_node = cur_node->left;
					else
						return insert_at(cur_node, cur_node->left);
				} else {
					// Go right or insert right
					if(cur_node->right)
						cur_node = cur_node->right;
					else
						return insert_at(cur_node, cur_node->right);
				}
			}
		}

		void erase(const Key& key) {
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
						cur_node->parent->update_height_and_balance(this);
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
								cur_node->update_height_and_balance(this);
							} else {
								successor->parent->left = successor->right;
								if(successor->parent->left)
									successor->parent->left->parent = successor->parent;
								successor->parent->update_height_and_balance(this);
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
							cur_node->update_height_and_balance(this);
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
								cur_node->update_height_and_balance(this);
							} else {
								predecessor->parent->right = predecessor->left;
								if(predecessor->parent->right)
									predecessor->parent->right->parent = predecessor->parent;
								predecessor->parent->update_height_and_balance(this);
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
							cur_node->update_height_and_balance(this);
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

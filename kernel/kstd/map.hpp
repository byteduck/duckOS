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
	template<typename Key, typename Val>
	class map {
	public:
		class Node;
	
		struct NodePair {
			Node* parent;
			Node* child;
		};
	
		class Node {
		public:
			Node(const pair<Key, Val>& data): data(data) {}

			pair<Key, Val> data;
			Node* left = nullptr;
			Node* right = nullptr;
			
			NodePair predecessor() {
				if(!left) {
					return {nullptr, nullptr};
				}
				auto* prev_node = this;
				auto* cur_node = left;
				while(cur_node->right) {
					prev_node = cur_node;
					cur_node = cur_node->right;
				}
				return {prev_node, cur_node};
			}
			
			NodePair successor() {
				if(!right) {
					return {nullptr, nullptr};
				}
				auto* prev_node = this;
				auto* cur_node = right;
				while(cur_node->left) {
					prev_node = cur_node;
					cur_node = cur_node->left;
				}
				return {prev_node, cur_node};
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
						m_size++;
						return cur_node->left;
					}
				} else {
					if(cur_node->right) {
						cur_node = cur_node->right;
					} else {
						cur_node->right = new Node(elem);
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
						auto successor_pair = cur_node->successor();
						if(successor_pair.child) {
							cur_node->data = successor_pair.child->data;
							if(successor_pair.parent == cur_node)
								successor_pair.parent->right = successor_pair.child->right;
							else
								successor_pair.parent->left = successor_pair.child->right;
							delete successor_pair.child;
						} else {
							// We don't have a successor - just replace the node with the left child.
							cur_node->data = cur_node->left->data;
							cur_node->right = cur_node->left->right;
							auto* old_left = cur_node->left;
							cur_node->left = cur_node->left->left;
							delete old_left;
						}
					} else {
						// If we're deleting the root or a node that's the left child, replace it with the predecessor
						auto predecessor_pair = cur_node->predecessor();
						if(predecessor_pair.child) {
							cur_node->data = predecessor_pair.child->data;
							if(predecessor_pair.parent == cur_node)
								predecessor_pair.parent->left = predecessor_pair.child->left;
							else
								predecessor_pair.parent->right = predecessor_pair.child->left;
							delete predecessor_pair.child;
						} else {
							// We don't have a predecessor - just replace the node with the right child.
							cur_node->data = cur_node->right->data;
							cur_node->left = cur_node->right->left;
							auto* old_right = cur_node->right;
							cur_node->right = cur_node->right->right;
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

	private:
		Node* m_root = nullptr;
		size_t m_size = 0;
	};
}

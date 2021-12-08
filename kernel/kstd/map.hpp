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

#ifndef DUCKOS_MAP_HPP
#define DUCKOS_MAP_HPP

#include "utility.h"
#include "pair.hpp"
#include "vector.hpp"

namespace kstd {
	template<typename Key, typename Val>
	class map {
	public:
		class leaf {
		public:
			leaf(const pair<Key, Val>& data): data(data) {}

			pair<Key, Val> data;
			leaf* left = nullptr;
			leaf* right = nullptr;
			bool exists = true;
		};

		map() = default;
		~map() {
			for(size_t i = 0; i < _leaves.size(); i++) {
				delete _leaves[i];
			}
		}

		leaf* find_leaf(const Key& key) const {
			if(!_root) {
				return nullptr;
			}

			auto* cur_leaf = _root;
			while(cur_leaf) {
				if(cur_leaf->data.first == key) {
					return cur_leaf;
				} else if(key < cur_leaf->data.first) {
					cur_leaf = cur_leaf->left;
				} else {
					cur_leaf = cur_leaf->right;
				}
			}

			return nullptr;
		}

		bool contains(const Key& key) const {
			auto* leaf = find_leaf(key);
			return leaf && leaf->exists;
		}

		leaf* insert(const pair<Key, Val>& elem) {
			//TODO: Balance on insertion
			if(!_root) {
				_root = new leaf(elem);
				_leaves.push_back(_root);
				return _root;
			}

			auto* cur_leaf = _root;
			while(true) {
				if(!cur_leaf->exists) {
					cur_leaf->exists = true;
					cur_leaf->data = elem;
					return cur_leaf;
				} else if(cur_leaf->data.first == elem.first) {
					return nullptr;
				} else if(elem.first < cur_leaf->data.first) {
					if(cur_leaf->left) {
						cur_leaf = cur_leaf->left;
					} else {
						cur_leaf->left = new leaf(elem);
						_leaves.push_back(cur_leaf->left);
						return cur_leaf->left;
					}
				} else {
					if(cur_leaf->right) {
						cur_leaf = cur_leaf->right;
					} else {
						cur_leaf->right = new leaf(elem);
						_leaves.push_back(cur_leaf->right);
						return cur_leaf->right;
					}
				}
			}
		}

		void erase(const Key& key) {
			auto* leaf = find_leaf(key);
			if(!leaf || !leaf->exists)
				return;
			leaf->data.second = Val();
			leaf->exists = false;
		}

		Val& operator[](const Key& key) {
			auto* ret = find_leaf(key);
			if(!ret || !ret->exists)
				return insert({key, Val()})->data.second;
			return ret->data.second;
		}

		const kstd::vector<leaf*> leaves() {
			return _leaves;
		}

	private:
		kstd::vector<leaf*> _leaves;
		leaf* _root = nullptr;
	};
}

#endif //DUCKOS_MAP_HPP
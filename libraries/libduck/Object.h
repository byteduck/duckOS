/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <memory>
#include <string>

#define DUCK_OBJECT_DEF(name) 										\
	template<class... ArgTs>										\
	static inline Duck::Ptr<name> make(ArgTs&&... args) {			\
		auto self = Duck::Ptr<name>(new name(args...));				\
		self->initialize();											\
        return self;												\
	}																\
	inline Duck::Ptr<name> self() {									\
		return std::static_pointer_cast<name>(shared_from_this());	\
	}																\
	template<typename T>											\
	inline Duck::Ptr<T> self() {									\
		return std::static_pointer_cast<T>(self());					\
	}                                      							\
	inline std::string object_name() { return #name; }

#define DUCK_OBJECT_VIRTUAL(name) 									\
	inline virtual std::string object_name() { return #name; }

namespace Duck {
	template<typename T>
	using Ptr = std::shared_ptr<T>;

	template<typename T>
	using PtrRef = const std::shared_ptr<T>&;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	class Object: public std::enable_shared_from_this<Object> {
	public:
		DUCK_OBJECT_DEF(Object)
		virtual void initialize();
		virtual ~Object() = default;
	};
}

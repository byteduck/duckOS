/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright © 2016-2022 Byteduck */

#pragma once

#include <memory>
#include <string>

/** \see{Duck::Object} */
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

/** \see{Duck::Object} */
#define DUCK_OBJECT_VIRTUAL(name) 									\
	inline virtual std::string object_name() { return #name; }

namespace Duck {
	template<typename T>
	using Ptr = std::shared_ptr<T>;

	template<typename T>
	using PtrRef = const std::shared_ptr<T>&;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	/**
	 * This class is to be used as a base class for various objects. To derive from Object, use the DUCK_OBJECT_DEF(T)
	 * macro in the public section of the class definition, where T is the name of your class. For pure virtual classes,
	 * use DUCK_OBJECT_VIRTUAL(T).
	 *
	 * Constructors for derived classes should be private, and the static `Ptr<T> make()` method should be used to
	 * construct new instances.
	 *
	 * The DUCK_OBJECT_DEF macro defines the following methods for your class T:
	 * - Ptr<T> make(args...): Constructs a new instance of your object with the same arguments as defined in your
	 *   private constructors.
	 * - Ptr<T> self(): Gets a reference to `this` as a smart pointer.
	 * - std::string object_name(): Gets the name of the object class ("T").
	 *
	 * The DUCK_OBJECT_VIRTUAL macro only defines `object_name`.
	 */
	class Object: public std::enable_shared_from_this<Object> {
	public:
		DUCK_OBJECT_DEF(Object)
		virtual ~Object() = default;

	protected:
		virtual void initialize();
	};
}

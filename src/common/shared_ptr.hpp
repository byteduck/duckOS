#ifndef DUCKOS_SHARED_PTR_HPP
#define DUCKOS_SHARED_PTR_HPP

/**
 * @file  shared_ptr.hpp
 * @brief shared_ptr is a minimal implementation of smart pointer, a subset of the C++11 std::shared_ptr or boost::shared_ptr.
 *
 * Copyright (c) 2013-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
 *
 * Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
 * or copy at http://opensource.org/licenses/MIT)
 */

#include <common/cstddef.h>
#include <common/utility.h>
#define SHARED_ASSERT(x) /*ASSERT(x)*/ //TODO: Fix


/**
 * @brief implementation of reference counter for the following minimal smart pointer.
 *
 * shared_ptr_count is a container for the allocated pn reference counter.
 */
namespace DC{
	class shared_ptr_count
	{
	public:
		shared_ptr_count() :
				pn(NULL)
		{
		}
		shared_ptr_count(const shared_ptr_count& count) :
				pn(count.pn)
		{
		}
		/// @brief Swap method for the copy-and-swap idiom (copy constructor and swap method)
		void swap(shared_ptr_count& lhs) throw() // never throws
		{
			DC::swap(pn, lhs.pn);
		}
		/// @brief getter of the underlying reference counter
		long use_count(void) const throw() // never throws
		{
			long count = 0;
			if (NULL != pn)
			{
				count = *pn;
			}
			return count;
		}
		/// @brief acquire/share the ownership of the pointer, initializing the reference counter
		template<class U>
		void acquire(U* p) // may throw std::bad_alloc
		{
			if (NULL != p)
			{
				if (NULL == pn)
				{
					pn = new long(1);
				}
				else
				{
					++(*pn);
				}
			}
		}
		/// @brief release the ownership of the px pointer, destroying the object when appropriate
		template<class U>
		void release(U* p) throw() // never throws
		{
			if (NULL != pn)
			{
				--(*pn);
				if (0 == *pn)
				{
					delete p;
					delete pn;
				}
				pn = NULL;
			}
		}

	public:
		long*   pn; //!< Reference counter
	};


/**
 * @brief minimal implementation of smart pointer, a subset of the C++11 std::shared_ptr or boost::shared_ptr.
 *
 * shared_ptr is a smart pointer retaining ownership of an object through a provided pointer,
 * and sharing this ownership with a reference counter.
 * It destroys the object when the last shared pointer pointing to it is destroyed or reset.
 */
	template<class T>
	class shared_ptr
	{
	public:
		/// The type of the managed object, aliased as member type
		typedef T element_type;

		/// @brief Default constructor
		shared_ptr(void) throw() : // never throws
				px(NULL),
				pn()
		{
		}
		/// @brief Constructor with the provided pointer to manage
		explicit shared_ptr(T* p) : // may throw std::bad_alloc
		//px(p), would be unsafe as acquire() may throw, which would call release() in destructor
				pn()
		{
			acquire(p);   // may throw std::bad_alloc
		}
		/// @brief Constructor to share ownership. Warning : to be used for pointer_cast only ! (does not manage two separate <T> and <U> pointers)
		template <class U>
		shared_ptr(const shared_ptr<U>& ptr, T* p) :
		//px(p), would be unsafe as acquire() may throw, which would call release() in destructor
				pn(ptr.pn)
		{
			acquire(p);   // may throw std::bad_alloc
		}
		/// @brief Copy constructor to convert from another pointer type
		template <class U>
		shared_ptr(const shared_ptr<U>& ptr) throw() : // never throws (see comment below)
		//px(ptr.px),
				pn(ptr.pn)
		{
			SHARED_ASSERT((NULL == ptr.px) || (0 != ptr.pn.use_count())); // must be coherent : no allocation allowed in this path
			acquire(static_cast<typename shared_ptr<T>::element_type*>(ptr.px));   // will never throw std::bad_alloc
		}
		/// @brief Copy constructor (used by the copy-and-swap idiom)
		shared_ptr(const shared_ptr& ptr) throw() : // never throws (see comment below)
		//px(ptr.px),
				pn(ptr.pn)
		{
			SHARED_ASSERT((NULL == ptr.px) || (0 != ptr.pn.use_count())); // must be cohérent : no allocation allowed in this path
			acquire(ptr.px);   // will never throw std::bad_alloc
		}
		/// @brief Assignment operator using the copy-and-swap idiom (copy constructor and swap method)
		shared_ptr& operator=(shared_ptr ptr) throw() // never throws
		{
			swap(ptr);
			return *this;
		}
		/// @brief the destructor releases its ownership
		~shared_ptr(void) throw() // never throws
		{
			release();
		}
		/// @brief this reset releases its ownership
		void reset(void) throw() // never throws
		{
			release();
		}
		/// @brief this reset release its ownership and re-acquire another one
		void reset(T* p) // may throw std::bad_alloc
		{
			SHARED_ASSERT((NULL == p) || (px != p)); // auto-reset not allowed
			release();
			acquire(p); // may throw std::bad_alloc
		}

		/// @brief Swap method for the copy-and-swap idiom (copy constructor and swap method)
		void swap(shared_ptr& lhs) throw() // never throws
		{
			DC::swap(px, lhs.px);
			pn.swap(lhs.pn);
		}

		// reference counter operations :
		operator bool() const throw() // never throws
		{
			return (0 < pn.use_count());
		}
		bool unique(void)  const throw() // never throws
		{
			return (1 == pn.use_count());
		}
		long use_count(void)  const throw() // never throws
		{
			return pn.use_count();
		}

		// underlying pointer operations :
		T& operator*()  const throw() // never throws
		{
			SHARED_ASSERT(NULL != px);
			return *px;
		}
		T* operator->() const throw() // never throws
		{
			SHARED_ASSERT(NULL != px);
			return px;
		}
		T* get(void)  const throw() // never throws
		{
			// no assert, can return NULL
			return px;
		}

	private:
		/// @brief acquire/share the ownership of the px pointer, initializing the reference counter
		void acquire(T* p) // may throw std::bad_alloc
		{
			pn.acquire(p); // may throw std::bad_alloc
			px = p; // here it is safe to acquire the ownership of the provided raw pointer, where exception cannot be thrown any more
		}

		/// @brief release the ownership of the px pointer, destroying the object when appropriate
		void release(void) throw() // never throws
		{
			pn.release(px);
			px = NULL;
		}

	private:
		// This allow pointer_cast functions to share the reference counter between different shared_ptr types
		template<class U>
		friend class shared_ptr;

	private:
		T*                  px; //!< Native pointer
		shared_ptr_count    pn; //!< Reference counter
	};


// comparaison operators
	template<class T, class U> bool operator==(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() == r.get());
	}
	template<class T, class U> bool operator!=(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() != r.get());
	}
	template<class T, class U> bool operator<=(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() <= r.get());
	}
	template<class T, class U> bool operator<(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() < r.get());
	}
	template<class T, class U> bool operator>=(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() >= r.get());
	}
	template<class T, class U> bool operator>(const shared_ptr<T>& l, const shared_ptr<U>& r) throw() // never throws
	{
		return (l.get() > r.get());
	}



// static cast of shared_ptr
	template<class T, class U>
	shared_ptr<T> static_pointer_cast(const shared_ptr<U>& ptr) // never throws
	{
		return shared_ptr<T>(ptr, static_cast<typename shared_ptr<T>::element_type*>(ptr.get()));
	}

// dynamic cast of shared_ptr
	template<class T, class U>
	shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& ptr) // never throws
	{
		T* p = dynamic_cast<typename shared_ptr<T>::element_type*>(ptr.get());
		if (NULL != p)
		{
			return shared_ptr<T>(ptr, p);
		}
		else
		{
			return shared_ptr<T>();
		}
	}

	template<typename T, class... Args>
	shared_ptr<T> make_shared(Args&&... args){
		return shared_ptr<T>(new T(args...));
	}
}

#endif //DUCKOS_SHARED_PTR_HPP

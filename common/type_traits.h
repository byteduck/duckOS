#ifndef DUCKOS_TYPE_TRAITS_H
#define DUCKOS_TYPE_TRAITS_H

namespace DC {
	template< class T > struct remove_reference      {typedef T type;};
	template< class T > struct remove_reference<T&>  {typedef T type;};
	template< class T > struct remove_reference<T&&> {typedef T type;};
}

#endif //DUCKOS_TYPE_TRAITS_H

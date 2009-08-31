#ifndef TYPET_HPP
#define TYPET_HPP

/**

Taken from the book
C++ Templates - The Complete Guide
by David Vandevoorde and Nicolai M. Josuttis, Addison-Wesley, 2002
© Copyright David Vandevoorde and Nicolai M. Josuttis 2002

*/

// define IsFundaT<>
/**********************************
 * type1.hpp:
 **********************************/
// primary template: in general T is no fundamental type
template <typename T>
class IsFundaT {
public:
	enum { Yes = 0, No = 1};
};

// macro to specialize for fundamental types
#define MK_FUNDA_TYPE(T)               \
    template<> class IsFundaT<T> {     \
      public:                          \
        enum { Yes = 1, No = 0 };      \
    };

MK_FUNDA_TYPE(void)

MK_FUNDA_TYPE(bool)
MK_FUNDA_TYPE(char)
MK_FUNDA_TYPE(signed char)
MK_FUNDA_TYPE(unsigned char)
MK_FUNDA_TYPE(wchar_t)

MK_FUNDA_TYPE(signed short)
MK_FUNDA_TYPE(unsigned short)
MK_FUNDA_TYPE(signed int)
MK_FUNDA_TYPE(unsigned int)
MK_FUNDA_TYPE(signed long)
MK_FUNDA_TYPE(unsigned long)
#if LONGLONG_EXISTS
  MK_FUNDA_TYPE(signed long long)
  MK_FUNDA_TYPE(unsigned long long)
#endif  // LONGLONG_EXISTS

MK_FUNDA_TYPE(float)
MK_FUNDA_TYPE(double)
MK_FUNDA_TYPE(long double)

#undef MK_FUNDA_TYPE
/**** end of type1.hpp ****/

// define primary template CompoundT<> (first version)
//#include "type2.hpp"

// define primary template CompoundT<> (second version)
/**********************************
 * type6.hpp:
 **********************************/

template<typename T>
class CompoundT {           // primary template
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 0,
           IsPtrMemT = 0 };
    typedef T BaseT;
    typedef T BottomT;
    typedef CompoundT<void> ClassT;
};
/**** end of type6.hpp ****/

// define CompoundT<> specializations
/**********************************
 * type3.hpp:
 **********************************/
template<typename T>
class CompoundT<T&> {       // partial specialization for references
  public:
    enum { IsPtrT = 0, IsRefT = 1, IsArrayT = 0, IsPtrMemT = 0 };
    typedef T BaseT;
    typedef typename CompoundT<T>::BottomT BottomT;
    typedef CompoundT<void> ClassT;
};

template<typename T>
class CompoundT<T*> {       // partial specialization for pointers
  public:
    enum { IsPtrT = 1, IsRefT = 0, IsArrayT = 0, IsPtrMemT = 0 };
    typedef T BaseT;
    typedef typename CompoundT<T>::BottomT BottomT;
    typedef CompoundT<void> ClassT;
};
/**** end of type3.hpp ****/
/**********************************
 * type4.hpp:
 **********************************/
#include <stddef.h>

template<typename T, size_t N>
class CompoundT <T[N]> {    // partial specialization for arrays
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 1, IsPtrMemT = 0 };
    typedef T BaseT;
    typedef typename CompoundT<T>::BottomT BottomT;
    typedef CompoundT<void> ClassT;
};

template<typename T>
class CompoundT <T[]> {    // partial specialization for empty arrays
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 1, IsPtrMemT = 0 };
    typedef T BaseT;
    typedef typename CompoundT<T>::BottomT BottomT;
    typedef CompoundT<void> ClassT;
};

template<typename T, typename C>
class CompoundT <T C::*> {  // partial specialization for pointer-to-members
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 0, IsPtrMemT = 1 };
    typedef T BaseT;
    typedef typename CompoundT<T>::BottomT BottomT;
    typedef C ClassT;
};
/**** end of type4.hpp ****/
/**********************************
 * type5.hpp:
 **********************************/
template<typename R>
class CompoundT<R()> {
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 0, IsPtrMemT = 0 };
    typedef R BaseT();
    typedef R BottomT();
    typedef CompoundT<void> ClassT;
};

template<typename R, typename P1>
class CompoundT<R(P1)> {
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 0, IsPtrMemT = 0 };
    typedef R BaseT(P1);
    typedef R BottomT(P1);
    typedef CompoundT<void> ClassT;
};

template<typename R, typename P1>
class CompoundT<R(P1, ...)> {
  public:
    enum { IsPtrT = 0, IsRefT = 0, IsArrayT = 0, IsPtrMemT = 0 };
    typedef R BaseT(P1);
    typedef R BottomT(P1);
    typedef CompoundT<void> ClassT;
};
//...
/**** end of type5.hpp ****/

// define IsEnumT<>
/**********************************
 * type7.hpp:
 **********************************/
struct SizeOverOne { char c[2]; };

template<typename T,
         bool convert_possible =
//          !CompoundT<T>::IsFuncT &&
          !CompoundT<T>::IsArrayT>
class ConsumeUDC {
  public:
    operator T() const;
};

// conversion to function and array types is not possible
template <typename T>
class ConsumeUDC<T, false> {
};

// conversion to void type is not possible
template <bool convert_possible>
class ConsumeUDC<void, convert_possible> {
};

char enum_check(bool);
char enum_check(char);
char enum_check(signed char);
char enum_check(unsigned char);
char enum_check(wchar_t);

char enum_check(signed short);
char enum_check(unsigned short);
char enum_check(signed int);
char enum_check(unsigned int);
char enum_check(signed long);
char enum_check(unsigned long);
#if LONGLONG_EXISTS
  char enum_check(signed long long);
  char enum_check(unsigned long long);
#endif  // LONGLONG_EXISTS

// avoid accidental conversions from float to int
char enum_check(float);
char enum_check(double);
char enum_check(long double);

SizeOverOne enum_check(...);    // catch all

template<typename T>
class IsEnumT {
  public:
    enum { Yes = IsFundaT<T>::No &&
                 !CompoundT<T>::IsRefT &&
                 !CompoundT<T>::IsPtrT &&
                 !CompoundT<T>::IsPtrMemT &&
                 sizeof(enum_check(ConsumeUDC<T>()))==1 };
    enum { No = !Yes };
};
/**** end of type7.hpp ****/

// define IsClassT<>
/**********************************
 * type8.hpp:
 **********************************/
template<typename T>
class IsClassT {
  public:
    enum {
    Yes = IsFundaT<T>::No
     && IsEnumT<T>::No
     && !CompoundT<T>::IsPtrT
     && !CompoundT<T>::IsRefT
     && !CompoundT<T>::IsArrayT
     && !CompoundT<T>::IsPtrMemT
//     && !CompoundT<T>::IsFuncT
    };
    enum { No = !Yes };
};
/**** end of type8.hpp ****/

// define template that handles all in one style
template <typename T>
class TypeT {
	public:
	enum { IsFundaT  = IsFundaT<T>::Yes,
		IsPtrT    = CompoundT<T>::IsPtrT,
		IsRefT    = CompoundT<T>::IsRefT,
		IsArrayT  = CompoundT<T>::IsArrayT,
		IsFuncT   = CompoundT<T>::IsFuncT,
		IsPtrMemT = CompoundT<T>::IsPtrMemT,
		IsEnumT   = IsEnumT<T>::Yes,
		IsClassT  = IsClassT<T>::Yes
	};
};

#endif // TYPET_HPP

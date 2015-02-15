#pragma once

#include <unordered_map>
#include <type_traits>

#include <Objidl.h>

/**
* here be dragons
*
* this is a rewrite of the old SwizzleManager
* because I thought it would be interesting and because the original code's approach to errors (execute a 1/0) is a WTF
*
* since the savegame contains a fuckton of pointers to other objects,
* after load those pointers need to be updated to point to the new versions of those objects
*  this system handles that: you use RegisterForChange(&ptr) to say "this is a pointer to a pointer that needs to be updated"
*  and RegisterChange(oldPtr, this) to say "what was at address oldPtr is now at address this"
*  once the loading is complete, ConvertNodes will go over the registered nodes and replace pointers it knows about
*/
class AresSwizzle {
protected:
	/**
	* data store for RegisterChange
	*/
	std::unordered_map<void *, void *> Changes;

	/**
	* data store for RegisterForChange
	*/
	std::unordered_multimap<void *, void **> Nodes;

public:
	static AresSwizzle Instance;

	AresSwizzle() {};
	~AresSwizzle() {};

	/**
	* pass in the *address* of the pointer you want to have changed
	* caution, after the call *p will be NULL
	*/
	HRESULT RegisterForChange(void **p);

	/**
	* the original game objects all save their `this` pointer to the save stream
	* that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
	*/
	HRESULT RegisterChange(void * was, void * is);

	/**
	* this function will rewrite all registered nodes' values
	*/
	void ConvertNodes() const;

	void Clear();

	template<typename T>
	void RegisterPointerForChange(T* &ptr) {
		auto pptr = const_cast<std::remove_cv_t<T>**>(&ptr);
		this->RegisterForChange(reinterpret_cast<void **>(pptr));
	};
};

template<typename T>
struct is_swizzlable : public std::is_pointer<T>::type {};

struct Swizzle {
	template <typename T>
	Swizzle(T &object) {
		swizzle(object, typename is_swizzlable<T>::type());
	}

private:
	template <typename TSwizzle>
	void swizzle(TSwizzle& object, std::true_type) {
		AresSwizzle::Instance.RegisterPointerForChange(object);
	}

	template <typename TSwizzle>
	void swizzle(TSwizzle& object, std::false_type) {
		// not swizzlable
	}
};

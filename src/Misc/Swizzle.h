#ifndef ARES_SWIZZLE_H
#define ARES_SWIZZLE_H

#include <xcompile.h>
#include <windows.h>
#include "Debug.h"
#include <ArrayClasses.h>

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
	hash_map<void *, void *> Changes;

	/**
	 * data store for RegisterForChange
	 */
	hash_multimap<void *, void **> Nodes;

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
	void RegisterPointerForChange(T &ptr);
};

template<typename T>
class ObjectSwizzler {
public:
	ObjectSwizzler(T &Object);
};

template<typename T>
void AresSwizzle::RegisterPointerForChange(T &ptr) {
	this->RegisterForChange(reinterpret_cast<void **>(&ptr));
};

template<typename T>
void ObjectSwizzler<T>::ObjectSwizzler(T &Object) {
	//nop
};

template<typename T>
void ObjectSwizzler<VectorClass<T> >::ObjectSwizzler(VectorClass<T> &Object) {
	for (auto ii = 0; ii < Object.Capacity; ++ii) {
		AresSwizzle::Instance.RegisterPointerForChange(&(Object.Items[ii]));
	}
};

template<typename T>
void ObjectSwizzler<DynamicVectorClass<T> >::ObjectSwizzler(DynamicVectorClass<T> &Object) {
	for (auto ii = 0; ii < Object.Count; ++ii) {
		AresSwizzle::Instance.RegisterPointerForChange(&(Object.Items[ii]));
	}
};

#endif

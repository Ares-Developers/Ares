#ifndef CONTAINER_TEMPLATE_MAGIC_H
#define CONTAINER_TEMPLATE_MAGIC_H

#include <hash_map>
#include <CCINIClass.h>
#include <MacroHelpers.h>

#include "..\Misc\Debug.h"

enum eInitState {
	is_Blank = 0x0, // CTOR'd
	is_Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	is_Ruled = 0x2, // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	is_Inited = 0x3, // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	is_Completed = 0x4 // INI has been read and values set
};

/*
 * ==========================
 *    It's a kind of magic   
 * ==========================

 * These two templates are the basis of the new class extension standard.

 * ==========================

 * Extension<T> is the parent class for the data you want to link with this instance of T
   ( for example, [Warhead]MindControl.Permanent= should be stored in WarheadClassExt::ExtData
     which itself should be a derivate of Extension<WarheadTypeClass> )

 * ==========================

   Container<TX> is the storage for all the Extension<T> which share the same T,
    where TX is the containing class of the relevant derivate of Extension<T>. // complex, huh?
   ( for example, there is Container<WarheadTypeExt> 
     which contains all the custom data for all WarheadTypeClass instances,
     and WarheadTypeExt itself contains just statics like the Container itself
      
      )

   Requires: 
   	typedef TX::TT = T;
   	const DWORD Extension<T>::Canary = (any dword value easily identifiable in a byte stream)
   	class TX::ExtData : public Extension<T> { custom_data; }

   Complex? Yes. That's partially why you should be happy these are premade for you.
 *
 */

template<typename T>
class Extension {
	public:
		eInitState _Initialized;
	#ifdef DEBUGBUILD
		DWORD SavedCanary;
	#endif

		static const DWORD Canary;

		Extension(const DWORD Canary = 0) : 
	#ifdef DEBUGBUILD
		SavedCanary(Canary), 
	#endif
		_Initialized(is_Blank)
		{ };

		virtual ~Extension() { };

		// use this implementation for all descendants
		// I mean it
		// sizeof(facepalm)
		// virtual size_t Size() const { return sizeof(*this); };
		virtual size_t Size() const = 0;

		//reimpl in each class separately
		virtual void LoadFromINI(T *pThis, CCINIClass *pINI) {};

		// for things that only logically work in rules - countries, sides, etc
		virtual void LoadFromRules(T *pThis, CCINIClass *pINI) {};
		virtual void InitializeConstants(T *pThis)
		{
			this->_Initialized = is_Constanted;
		};
		virtual void InitializeRuled(T *pThis)
		{
			this->_Initialized = is_Ruled;
		};
		virtual void Initialize(T *pThis)
		{
			this->_Initialized = is_Inited;
		};
};

//template<typename T1, typename T2>
template<typename T>
class Container : public stdext::hash_map<typename T::TT*, typename T::ExtData* > {
private:
	typedef typename T::TT               S_T;
	typedef typename T::ExtData          E_T;
	typedef stdext::hash_map<S_T*, E_T*> C_Map;

public:
	Container() {
	}

	virtual ~Container() {
		Empty();
	}

	E_T *FindOrAllocate(S_T* key) {
		if(key == NULL) {
			DEBUGLOG("CTOR of Ext attempted for a NULL pointer! WTF!\n");
			return NULL;
		}
		C_Map::iterator i = find(key);
		if(i == end()) {
			DEBUGLOG("CTOR of Ext for %X (%s) ... ", key, key->get_ID());
			E_T * val = new E_T(typename E_T::Canary);
			val->InitializeConstants(key);
			i = insert(C_Map::value_type(key, val)).first;
			DEBUGLOG("Cntr len now %d\n", size());
		}
		return i->second;
	}

	E_T *Find(S_T* key) {
		C_Map::iterator i = find(key);
		DEBUGLOG("Lookup of Ext for %X (%s) ... ", key, key->get_ID());
		if(i == end()) {
			DEBUGLOG("Failed!\n");
			return NULL;
		}
		DEBUGLOG("Success!\n");
		return i->second;
	}

	void Remove(S_T* key) {
		C_Map::iterator i = find(key);
		DEBUGLOG("DTOR of Ext for %X (%s) ... ", key, key->get_ID());
		if(i != end()) {
			delete i->second;
			erase(i);
		}
		DEBUGLOG("Cntr len now %d\n", size());
	}

	void Empty() {
		for(C_Map::iterator i = begin(); i != end(); ) {
			delete i->second;
			erase(i++);
	//		++i;
		}
	}

	void LoadAllFromINI(CCINIClass *pINI) {
		for(C_Map::iterator i = begin(); i != end(); i++) {
			i->second->LoadFromINI(i->first, pINI);
		}
	}

	void LoadFromINI(S_T*key, CCINIClass *pINI) {
		C_Map::iterator i = find(key);
		if(i != end()) {
			i->second->LoadFromINI(key, pINI);
		}
	}

	void LoadAllFromRules(CCINIClass *pINI) {
		for(C_Map::iterator i = begin(); i != end(); i++) {
			i->second->LoadFromRules(i->first, pINI);
		}
	}

	void Save(S_T *key, IStream *pStm) {
		ULONG out;
		E_T* buffer = this->Find(key);
		if(buffer) {
			pStm->Write(buffer, buffer->Size(), &out);
		}
	};

	void Load(S_T *key, IStream *pStm) {
		ULONG out;
		E_T* buffer = this->FindOrAllocate(key); //OrAllocate, of course
//		if(buffer) {
			pStm->Read(buffer, buffer->Size(), &out);
#ifdef DEBUGBUILD
			assert(buffer->SavedCanary == typename E_T::Canary);
#endif
//		}
	};

};

#endif

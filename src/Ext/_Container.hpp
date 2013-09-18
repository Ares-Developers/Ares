#ifndef CONTAINER_TEMPLATE_MAGIC_H
#define CONTAINER_TEMPLATE_MAGIC_H

#include <typeinfo>

#include <xcompile.h>
#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include "../Misc/Debug.h"

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
		T* const AttachedToObject;
	#ifdef DEBUGBUILD
		DWORD SavedCanary;
	#endif

		static const DWORD Canary;

		Extension(const DWORD Canary, T* const OwnerObject) :
		_Initialized(is_Blank),
	#ifdef DEBUGBUILD
		SavedCanary(Canary),
	#endif
		AttachedToObject(OwnerObject)
		{ };

		virtual ~Extension() { };

		// use this implementation for all descendants
		// I mean it
		// sizeof(facepalm)
		// virtual size_t Size() const { return sizeof(*this); };
		virtual size_t Size() const = 0;

		// major refactoring!
		// LoadFromINI is now a non-virtual public function that orchestrates the initialization/loading of extension data
		// all its slaves are now protected functions
		void LoadFromINI(T *pThis, CCINIClass *pINI) {
			if(!pINI) {
				return;
			}

			switch(this->_Initialized) {
				case is_Blank:
					this->InitializeConstants(pThis);
					this->_Initialized = is_Constanted;
				case is_Constanted:
					this->InitializeRuled(pThis);
					this->_Initialized = is_Ruled;
				case is_Ruled:
					this->Initialize(pThis);
					this->_Initialized = is_Inited;
				case is_Inited:
				case is_Completed:
					if(pINI == CCINIClass::INI_Rules) {
						this->LoadFromRulesFile(pThis, pINI);
					}
					this->LoadFromINIFile(pThis, pINI);
					this->_Initialized = is_Completed;
			}
		}

//	protected:
		//reimpl in each class separately
		virtual void LoadFromINIFile(T *pThis, CCINIClass *pINI) {};

		// for things that only logically work in rules - countries, sides, etc
		virtual void LoadFromRulesFile(T *pThis, CCINIClass *pINI) {};

		virtual void InitializeConstants(T *pThis) { };

		virtual void InitializeRuled(T *pThis) { };

		virtual void Initialize(T *pThis) { };

		virtual void InvalidatePointer(void *ptr, bool bRemoved) = 0;

	private:
		void operator = (Extension &RHS) {

		}
};

//template<typename T1, typename T2>
template<typename T>
class Container : public hash_map<typename T::TT*, typename T::ExtData* > {
private:
	typedef typename T::TT        S_T;
	typedef typename T::ExtData   E_T;
	typedef S_T* KeyType;
	typedef E_T* ValueType;
	typedef hash_map<KeyType, ValueType> C_Map;

public:
	static S_T * SavingObject;
	static IStream * SavingStream;

	void PointerGotInvalid(void *ptr, bool bRemoved) {
		this->InvalidatePointer(ptr, bRemoved);
		this->InvalidateExtDataPointer(ptr, bRemoved);
	}

protected:
	// invalidate pointers to container's static gunk here (use full qualified names)
	virtual void InvalidatePointer(void *ptr, bool bRemoved) {
	};

	void InvalidateExtDataPointer(void *ptr, bool bRemoved) {
		for(auto i = this->begin(); i != this->end(); ++i) {
			i->second->InvalidatePointer(ptr, bRemoved);
		}
	}

public:
	Container() : hash_map<KeyType, ValueType>() {
	}

	virtual ~Container() {
		Empty();
	}

	ValueType FindOrAllocate(KeyType const &key) {
		if(key == NULL) {
			const auto &info = typeid(*this);
			Debug::Log("CTOR of %s attempted for a NULL pointer! WTF!\n", info.name());
			return NULL;
		}
		auto i = this->find(key);
		if(i == this->end()) {
			auto val = new E_T(E_T::Canary, key);
			val->InitializeConstants(key);
			i = this->insert(typename C_Map::value_type(key, val)).first;
		}
		return i->second;
	}

	ValueType Find(const KeyType &key) {
		auto i = this->find(key);
		if(i == this->end()) {
			return NULL;
		}
		return i->second;
	}

	const ValueType Find(const KeyType &key) const {
		auto i = this->find(key);
		if(i == this->end()) {
			return NULL;
		}
		return i->second;
	}

	void Remove(KeyType key) {
		auto i = this->find(key);
		if(i != this->end()) {
			delete i->second;
			erase(i);
		}
	}

	void Remove(typename C_Map::iterator i) {
		if(i != this->end()) {
			delete i->second;
			erase(i);
		}
	}

	void Empty() {
		for(auto i = this->begin(); i != this->end(); ) {
			delete i->second;
			erase(i++);
	//		++i;
		}
	}

	void LoadAllFromINI(CCINIClass *pINI) {
		for(auto i = this->begin(); i != this->end(); i++) {
			i->second->LoadFromINI(i->first, pINI);
		}
	}

	void LoadFromINI(KeyType key, CCINIClass *pINI) {
		auto i = this->find(key);
		if(i != this->end()) {
			i->second->LoadFromINI(key, pINI);
		}
	}

	void LoadAllFromRules(CCINIClass *pINI) {
		for(auto i = this->begin(); i != this->end(); i++) {
			i->second->LoadFromRulesFile(i->first, pINI);
		}
	}

	void SaveStatic() {
		if(Container<T>::SavingObject && Container<T>::SavingStream) {
			this->Save(Container<T>::SavingObject, Container<T>::SavingStream);
			Container<T>::SavingObject = NULL;
			Container<T>::SavingStream = NULL;
		}
	}

	void Save(KeyType key, IStream *pStm) {
		this->SaveKey(key, pStm);
	}

	ValueType SaveKey(KeyType key, IStream *pStm) {
		ULONG out;

		const auto &info = typeid(key);
		Debug::Log("Saving Key [%s] (%X)\n", info.name(), key);

		if(key == NULL) {
			return NULL;
		}
		auto buffer = this->Find(key);
		Debug::Log("\tKey maps to %X\n", buffer);
		if(buffer) {
			pStm->Write(&buffer, 4, &out);
			pStm->Write(buffer, buffer->Size(), &out);
//			Debug::Log("Save used up 0x%X bytes (HRESULT 0x%X)\n", out, res);
		}

		Debug::Log("\n\n");
		return buffer;
	};

	void LoadStatic() {
		if(Container<T>::SavingObject && Container<T>::SavingStream) {
			this->Load(Container<T>::SavingObject, Container<T>::SavingStream);
			Container<T>::SavingObject = NULL;
			Container<T>::SavingStream = NULL;
		}
	}

	void Load(KeyType key, IStream *pStm) {
		this->LoadKey(key, pStm);
	}

	ValueType LoadKey(KeyType key, IStream *pStm) {
		ULONG out;

		const auto &info = typeid(key);
		Debug::Log("Loading Key [%s] (%X)\n", info.name(), key);

		if(key == NULL) {
			Debug::Log("Load attempted for a NULL pointer! WTF!\n");
			return NULL;
		}
		auto buffer = this->FindOrAllocate(key);
		long origPtr;
		pStm->Read(&origPtr, 4, &out);
		pStm->Read(buffer, buffer->Size(), &out);
		Debug::Log("LoadKey Swizzle: %X => %X\n", origPtr, buffer);
		SwizzleManagerClass::Instance.Here_I_Am(origPtr, (void *)buffer);
		SWIZZLE(buffer->AttachedToObject);
#ifdef DEBUGBUILD
			assert(buffer->SavedCanary == typename E_T::Canary);
#endif
		return buffer;
	};
};

#endif

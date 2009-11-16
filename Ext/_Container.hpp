#ifndef CONTAINER_TEMPLATE_MAGIC_H
#define CONTAINER_TEMPLATE_MAGIC_H

#include <typeinfo>

#include <hash_map>
#include <CCINIClass.h>
#include <SwizzleManagerClass.h>
#include <Helpers/Macro.h>

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
		const T* AttachedToObject;
	#ifdef DEBUGBUILD
		DWORD SavedCanary;
	#endif

		static const DWORD Canary;

		Extension(const DWORD Canary = 0, const T* OwnerObject = NULL) :
		AttachedToObject(OwnerObject),
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
				case is_Constanted:
					this->InitializeRuled(pThis);
				case is_Ruled:
					this->Initialize(pThis);
				case is_Inited:
					if(pINI == CCINIClass::INI_Rules) {
						this->LoadFromRulesFile(pThis, pINI);
					}
					this->LoadFromINIFile(pThis, pINI);
			}
		}

//	protected:
		//reimpl in each class separately
		virtual void LoadFromINIFile(T *pThis, CCINIClass *pINI) {};

		// for things that only logically work in rules - countries, sides, etc
		virtual void LoadFromRulesFile(T *pThis, CCINIClass *pINI) {};

		virtual void InitializeConstants(T *pThis) {
			this->_Initialized = is_Constanted;
		};

		virtual void InitializeRuled(T *pThis) {
			this->_Initialized = is_Ruled;
		};

		virtual void Initialize(T *pThis) {
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

	unsigned int CTOR_Count;
	unsigned int DTOR_Count;
	unsigned int Lookup_Failure_Count;
	unsigned int Lookup_Success_Count;

public:
	static S_T * SavingObject;
	static IStream * SavingStream;

	Container() :
		CTOR_Count(0),
		DTOR_Count(0),
		Lookup_Failure_Count(0),
		Lookup_Success_Count(0)
	{
	}

	virtual ~Container() {
		Empty();
	}

	E_T *FindOrAllocate(S_T* key) {
		if(key == NULL) {
			const std::type_info &info = typeid(*this);
			Debug::Log("CTOR of %s attempted for a NULL pointer! WTF!\n", info.name());
			return NULL;
		}
		C_Map::iterator i = find(key);
		if(i == end()) {
			++CTOR_Count;
			E_T * val = new E_T(typename E_T::Canary, key);
			val->InitializeConstants(key);
			i = insert(C_Map::value_type(key, val)).first;
		}
		return i->second;
	}

	E_T *Find(S_T* key) {
		C_Map::iterator i = find(key);
		if(i == end()) {
			++Lookup_Failure_Count;
			return NULL;
		}
		++Lookup_Success_Count;
		return i->second;
	}

	void Remove(S_T* key) {
		C_Map::iterator i = find(key);
		if(i != end()) {
			delete i->second;
			erase(i);
			++DTOR_Count;
		}
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

	void Save(S_T *key, IStream *pStm) {
		this->SaveKey(key, pStm);
	}

	E_T* SaveKey(S_T *key, IStream *pStm) {
		ULONG out;
		
		const std::type_info &info = typeid(key);
		Debug::Log("Saving Key [%s] (%X)\n", info.name(), key);

		if(key == NULL) {
			return NULL;
		}
		E_T* buffer = this->Find(key);
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

	void Load(S_T *key, IStream *pStm) {
		this->LoadKey(key, pStm);
	}

	E_T* LoadKey(S_T *key, IStream *pStm) {
		ULONG out;

		const std::type_info &info = typeid(key);
		Debug::Log("Loading Key [%s] (%X)\n", info.name(), key);

		if(key == NULL) {
			Debug::Log("Load attempted for a NULL pointer! WTF!\n");
			return NULL;
		}
		E_T* buffer = this->FindOrAllocate(key);
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

	void Stat() {
		const std::type_info &info = typeid(*this);
		Debug::Log("Stats for container %s:\n", info.name());

		Debug::Log("|%08d|%08d|%08d/%08d|%08d|%08d|\n", 
			CTOR_Count, DTOR_Count, Lookup_Success_Count, Lookup_Failure_Count, size(), S_T::Array->Count);
	}

};

#endif

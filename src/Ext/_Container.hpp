#ifndef CONTAINER_TEMPLATE_MAGIC_H
#define CONTAINER_TEMPLATE_MAGIC_H

#ifdef _MSC_VER
#include <typeinfo>
#endif

#ifdef __GNUC__
#include <ext/hash_fun.h>
namespace __gnu_cxx {
	template<>
	struct hash<void *> {
		size_t operator()(void* const &v) const {
			return reinterpret_cast<std::size_t>(v);
		}
	};
};
#endif

#include <new>
#include <xcompile.h>
#include <CCINIClass.h>
#include "../Misc/Swizzle.h"
#include "../Misc/Savegame.h"
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
   	class TX::ExtData : public Extension<T> { custom_data; }

   Complex? Yes. That's partially why you should be happy these are premade for you.
 *
 */

template<typename T>
class Extension {
	public:
		eInitState _Initialized;
		T* AttachedToObject;

		Extension(T* const OwnerObject) :
			_Initialized(is_Blank),
			AttachedToObject(OwnerObject)
		{ };

		virtual ~Extension() { };

		/**
		 * this function should determine how many bytes are needed to write this object to the savegame
		 * should sum up
		 */
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

		virtual void InvalidatePointer(void *ptr) = 0;

		virtual void SaveToStream(AresSaveStream &pStm) {
			AresSwizzle::SaveToStream(pStm, this->_Initialized);
			AresSwizzle::SaveToStream(pStm, this->AttachedToObject);
		};

		virtual void LoadFromFile(IStream *pStm, size_t Size, size_t &Offset) {
			AresSwizzle::LoadFromFile(pStm, this->_Initialized, Size, Offset);
			AresSwizzle::LoadFromFile(pStm, this->AttachedToObject, Size, Offset);
		};

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
	typedef hash_map<S_T*, E_T*>  C_Map;

public:
	static S_T * SavingObject;
	static IStream * SavingStream;

	void PointerGotInvalid(void *ptr) {
		this->InvalidatePointer(ptr);
		this->InvalidateExtDataPointer(ptr);
	}

#define INVALID_CTR(type, p) \
	type::ExtMap.PointerGotInvalid(p);

protected:
	// invalidate pointers to container's static gunk here (use full qualified names)
	virtual void InvalidatePointer(void *ptr) {
	};

	void InvalidateExtDataPointer(void *ptr) {
		for(typename C_Map::iterator i = this->begin(); i != this->end(); ++i) {
			i->second->InvalidatePointer(ptr);
		}
	}

public:
	Container() {
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
		typename C_Map::iterator i = this->find(key);
		if(i == this->end()) {
			E_T * val = new E_T(key);
			val->InitializeConstants(key);
			i = this->insert(typename C_Map::value_type(key, val)).first;
		}
		return i->second;
	}

	E_T *Find(S_T* key) {
		typename C_Map::iterator i = this->find(key);
		if(i == this->end()) {
			return NULL;
		}
		return i->second;
	}

	void Remove(S_T* key) {
		typename C_Map::iterator i = this->find(key);
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
		for(typename C_Map::iterator i = this->begin(); i != this->end(); ) {
			delete i->second;
			erase(i++);
	//		++i;
		}
	}

	void LoadAllFromINI(CCINIClass *pINI) {
		for(typename C_Map::iterator i = this->begin(); i != this->end(); i++) {
			i->second->LoadFromINI(i->first, pINI);
		}
	}

	void LoadFromINI(S_T*key, CCINIClass *pINI) {
		typename C_Map::iterator i = this->find(key);
		if(i != this->end()) {
			i->second->LoadFromINI(key, pINI);
		}
	}

	void LoadAllFromRules(CCINIClass *pINI) {
		for(typename C_Map::iterator i = this->begin(); i != this->end(); i++) {
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
			AresSaveStream saver(buffer->Size());
			buffer->SaveToStream(saver);
			size_t sz = saver.size();
			pStm->Write(&sz, sizeof(sz), &out);
			pStm->Write(saver.data(), sz, &out);
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
		void *origPtr;
		pStm->Read(&origPtr, 4, &out);
		size_t sz;
		pStm->Read(&sz, 4, &out);
		size_t offset(0);

		/**
		 * this is a neat solution to class sizes mutating over time
		 * sz indicates how many bytes were originally written into the stream
		 * offset will be a running counter reading how many bytes were read
		 */

		buffer->LoadFromFile(pStm, sz, offset);
		if(offset != sz) {
			Debug::Log("LoadKey read %X bytes instead of %X!\n", offset, sz);
		}
		Debug::Log("LoadKey Swizzle: %X => %X\n", origPtr, buffer);
		AresSwizzle::Instance.RegisterChange(origPtr, (void *)buffer);
		AresSwizzle::Instance.RegisterPointerForChange(buffer->AttachedToObject);
		return buffer;
	};
};

#endif

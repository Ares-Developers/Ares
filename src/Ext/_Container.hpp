#ifndef CONTAINER_TEMPLATE_MAGIC_H
#define CONTAINER_TEMPLATE_MAGIC_H

#include <typeinfo>
#include <memory>
#include <unordered_map>

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include "../Misc/Debug.h"
#include "../Misc/Stream.h"
#include "../Misc/Swizzle.h"

enum class InitState {
	Blank = 0x0, // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2, // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3, // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4 // INI has been read and values set
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
	InitState Initialized;
	T* AttachedToObject;
public:

	static const DWORD Canary;

	Extension(T* const OwnerObject) :
		Initialized(InitState::Blank),
		AttachedToObject(OwnerObject)
	{ };

	Extension(const Extension &other) = delete;

	void operator = (const Extension &RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* const& OwnerObject() const {
		return this->AttachedToObject;
	}

	void EnsureConstanted() {
		if(this->Initialized < InitState::Constanted) {
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI) {
		if(!pINI) {
			return;
		}

		switch(this->Initialized) {
		case InitState::Blank:
			this->EnsureConstanted();
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
		case InitState::Inited:
		case InitState::Completed:
			if(pINI == CCINIClass::INI_Rules) {
				this->LoadFromRulesFile(pINI);
			}
			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual void InvalidatePointer(void* ptr, bool bRemoved) = 0;

	virtual inline void SaveToStream(AresStreamWriter &Stm) {
		Stm.Save(this->Initialized);
		//Stm.Save(this->AttachedToObject);
	}

	virtual inline void LoadFromStream(AresStreamReader &Stm) {
		Stm.Load(this->Initialized);
		//Stm.Load(this->AttachedToObject);
	}

protected:
	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() { };

	virtual void InitializeRuled() { };

	// called before the first ini file is read
	virtual void Initialize() { };

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) { };

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) { };

};

template<typename T>
class Container {
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using key_type = base_type*;
	using const_key_type = const base_type*;
	using value_type = extension_type*;
	using map_type = std::unordered_map<const_key_type, std::unique_ptr<extension_type>>;

	map_type Items;

	base_type * SavingObject;
	IStream * SavingStream;

public:
	void PointerGotInvalid(void *ptr, bool bRemoved) {
		this->InvalidatePointer(ptr, bRemoved);
		this->InvalidateExtDataPointer(ptr, bRemoved);
	}

protected:
	// invalidate pointers to container's static gunk here (use full qualified names)
	virtual void InvalidatePointer(void *ptr, bool bRemoved) {
	};

	void InvalidateExtDataPointer(void *ptr, bool bRemoved) {
		for(const auto& i : this->Items) {
			if(auto& value = i.second) {
				value->InvalidatePointer(ptr, bRemoved);
			}
		}
	}

public:
	Container() : Items() {
	}

	virtual ~Container() = default;

	value_type FindOrAllocate(key_type key) {
		if(key == nullptr) {
			const auto &info = typeid(*this);
			Debug::Log("CTOR of %s attempted for a NULL pointer! WTF!\n", info.name());
			return nullptr;
		}
		auto i = this->Items.find(key);
		if(i == this->Items.end()) {
			auto val = std::make_unique<extension_type>(key);
			val->EnsureConstanted();
			i = this->Items.emplace(key, std::move(val)).first;
		}
		return i->second.get();
	}

	value_type Find(const_key_type key) const {
		auto i = this->Items.find(key);
		if(i == this->Items.end()) {
			return nullptr;
		}
		return i->second.get();
	}

	void Remove(const_key_type key) {
		auto i = this->Items.find(key);
		if(i != this->Items.end()) {
			this->Items.erase(i);
		}
	}

	void Clear() {
		if(!this->Items.empty()) {
			const auto &info = typeid(*this);
			Debug::DevLog(Debug::Warning, "Cleared %u items from %s.\n",
				this->Items.size(), info.name());
		}
		this->Items.clear();
	}

	void LoadAllFromINI(CCINIClass *pINI) {
		for(const auto& i : this->Items) {
			i.second->LoadFromINI(pINI);
		}
	}

	void LoadFromINI(const_key_type key, CCINIClass *pINI) {
		auto i = this->Items.find(key);
		if(i != this->Items.end()) {
			i->second->LoadFromINI(pINI);
		}
	}

	void PrepareStream(key_type key, IStream *pStm) {
		const auto &info = typeid(base_type);
		Debug::Log("[PrepareStream] Next is %p of type '%s'\n", key, info.name());

		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	void SaveStatic() {
		const auto &info = typeid(base_type);

		if(this->SavingObject && this->SavingStream) {
			Debug::Log("[SaveStatic] Saving object %p as '%s'\n", this->SavingObject, info.name());

			if(!this->Save(this->SavingObject, this->SavingStream)) {
				Debug::FatalErrorAndExit("[SaveStatic] Saving failed!\n");
			}
		} else {
			Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
				info.name(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic() {
		const auto &info = typeid(base_type);

		if(this->SavingObject && this->SavingStream) {
			Debug::Log("[LoadStatic] Loading object %p as '%s'\n", this->SavingObject, info.name());

			if(!this->Load(this->SavingObject, this->SavingStream)) {
				Debug::FatalErrorAndExit("[LoadStatic] Loading failed!\n");
			}
		} else {
			Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
				info.name(), this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

protected:
	// specialize this method to do type-specific stuff
	bool Save(key_type key, IStream *pStm) {
		return this->SaveKey(key, pStm) != nullptr;
	}

	// specialize this method to do type-specific stuff
	bool Load(key_type key, IStream *pStm) {
		return this->LoadKey(key, pStm) != nullptr;
	}

	value_type SaveKey(key_type key, IStream *pStm) {
		// this really shouldn't happen
		if(!key) {
			Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		auto buffer = this->Find(key);
		if(!buffer) {
			Debug::Log("[SaveKey] Could not find value.\n");
			return nullptr;
		}

		// write the current pointer, the size of the block, and the canary
		AresByteStream saver(sizeof(*buffer));
		AresStreamWriter writer(saver);

		writer.Save(extension_type::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if(!saver.WriteBlockToStream(pStm)) {
			Debug::Log("[SaveKey] Failed to save data.\n");
			return nullptr;
		}

		Debug::Log("[SaveKey] Save used up 0x%X bytes\n", saver.Size());

		// done
		return buffer;
	};

	value_type LoadKey(key_type key, IStream *pStm) {
		// this really shouldn't happen
		if(!key) {
			Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
			return nullptr;
		}

		// get the value data
		auto buffer = this->FindOrAllocate(key);
		if(!buffer) {
			Debug::Log("[LoadKey] Could not find or allocate value.\n");
			return nullptr;
		}

		AresByteStream loader(0);
		if(!loader.ReadBlockFromStream(pStm)) {
			Debug::Log("[LoadKey] Failed to read data from save stream?!\n");
			return nullptr;
		}

		AresStreamReader reader(loader);
		if(reader.Expect(extension_type::Canary) && reader.RegisterChange(buffer)) {
			buffer->LoadFromStream(reader);
			if(reader.ExpectEndOfBlock()) {
				return buffer;
			}
		}

		return nullptr;
	};
};

#endif

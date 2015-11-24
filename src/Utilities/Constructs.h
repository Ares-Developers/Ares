#pragma once

#include <Theater.h>
#include <CCINIClass.h>
#include <GeneralStructures.h>
#include <StringTable.h>
#include <Helpers/String.h>
#include <PCX.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include "../Ares.h"
#include "../Ares.CRT.h"

#include "../Misc/Savegame.h"

class ConvertClass;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

struct Leptons {
	Leptons() = default;
	explicit Leptons(int value) noexcept : value(value) {}

	operator int() const {
		return this->value;
	}

	int value{ 0 };
};

class CustomPalette {
public:
	enum class PaletteMode : unsigned int {
		Default = 0,
		Temperate = 1
	};

	PaletteMode Mode{ PaletteMode::Default };
	UniqueGamePtr<ConvertClass> Convert{ nullptr };
	UniqueGamePtr<BytePalette> Palette{ nullptr };

	CustomPalette() = default;
	explicit CustomPalette(PaletteMode mode) noexcept : Mode(mode) {};

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	bool LoadFromINI(
		CCINIClass* pINI, const char* pSection, const char* pKey,
		const char* pDefault = "");

	bool Load(AresStreamReader &Stm, bool RegisterForChange);
	bool Save(AresStreamWriter &Stm) const;

private:
	void Clear();
	void CreateConvert();
};

// vector of char* with builtin storage
class VectorNames {
protected:
	DynamicVectorClass<const char*> Strings;
	char* Buffer{ nullptr };

public:
	VectorNames() = default;

	VectorNames(const char* pBuffer) {
		this->Tokenize(pBuffer);
	}

	~VectorNames() {
		this->Clear();
	}

	const char* operator[] (int index) const {
		return this->Strings.GetItemOrDefault(index);
	}

	const DynamicVectorClass<const char*>& Entries() const {
		return this->Strings;
	}

	const char** ToString() const {
		return this->Strings.Items;
	}

	int Count() const {
		return this->Strings.Count;
	}

	void Clear() {
		if(this->Buffer) {
			this->Strings.Clear();
			free(this->Buffer);
			this->Buffer = nullptr;
		}
	}

	void Tokenize() {
		if(this->Buffer) {
			this->Strings.Clear();

			char* context = nullptr;
			for(auto cur = strtok_s(this->Buffer, ",", &context); cur && *cur; cur = strtok_s(nullptr, ",", &context)) {
				this->Strings.AddItem(cur);
			}
		}
	}

	void Tokenize(const char* pBuffer) {
		if(pBuffer) {
			this->Clear();
			this->Buffer = _strdup(pBuffer);
			this->Tokenize();
		}
	}
};

// a poor man's map with contiguous storage
template <typename TKey, typename TValue>
class AresMap {
public:
	TValue& operator[] (const TKey& key) {
		if(auto pValue = this->find(key)) {
			return *pValue;
		}
		return this->insert_unchecked(key, TValue());
	}

	TValue* find(const TKey& key) {
		auto pValue = static_cast<const AresMap*>(this)->find(key);
		return const_cast<TValue*>(pValue);
	}

	const TValue* find(const TKey& key) const {
		auto it = this->get_iterator(key);
		if(it != this->values.end()) {
			return &it->second;
		}
		return nullptr;
	}

	TValue get_or_default(const TKey& key) const {
		if(auto pValue = this->find(key)) {
			return *pValue;
		}
		return TValue();
	}

	TValue get_or_default(const TKey& key, TValue def) const {
		if(auto pValue = this->find(key)) {
			return *pValue;
		}
		return def;
	}

	bool erase(const TKey& key) {
		auto it = this->get_iterator(key);
		if(it != this->values.end()) {
			this->values.erase(it);
			return true;
		}
		return false;
	}

	bool contains(const TKey& key) const {
		return this->get_iterator(key) != this->values.end();
	}

	bool insert(const TKey& key, TValue value) {
		if(!this->find(key)) {
			this->insert_unchecked(key, std::move(value));
			return true;
		}
		return false;
	}

	size_t size() const {
		return this->values.size();
	}

	bool empty() const {
		return this->values.empty();
	}

	void clear() {
		this->values.clear();
	}

	bool load(AresStreamReader &Stm, bool RegisterForChange) {
		this->clear();

		size_t size = 0;
		auto ret = Stm.Load(size);

		if(ret && size) {
			this->values.resize(size);
			for(size_t i = 0; i < size; ++i) {
				if(!Savegame::ReadAresStream(Stm, this->values[i].first, RegisterForChange)
					|| !Savegame::ReadAresStream(Stm, this->values[i].second, RegisterForChange))
				{
					return false;
				}
			}
		}

		return ret;
	}

	bool save(AresStreamWriter &Stm) const {
		Stm.Save(this->values.size());

		for(const auto& item : this->values) {
			Savegame::WriteAresStream(Stm, item.first);
			Savegame::WriteAresStream(Stm, item.second);
		}

		return true;
	}

private:
	using container_t = std::vector<std::pair<TKey, TValue>>;

	typename container_t::const_iterator get_iterator(const TKey& key) const {
		return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item) {
			return item.first == key;
		});
	}

	TValue& insert_unchecked(const TKey& key, TValue value) {
		this->values.emplace_back(key, std::move(value));
		return this->values.back().second;
	}

	container_t values;
};

// pcx filename storage with optional automatic loading
class AresPCXFile {
	static const size_t Capacity = 0x20;
public:
	explicit AresPCXFile(bool autoResolve = true) : filename(), resolve(autoResolve), checked(false), exists(false) {
	}

	AresPCXFile(const char* pFilename, bool autoResolve = true) : AresPCXFile(autoResolve) {
		*this = pFilename;
	}

	AresPCXFile& operator = (const char* pFilename) {
		this->filename = pFilename;
		auto& data = this->filename.data();
		_strlwr_s(data);

		this->checked = false;
		this->exists = false;

		if(this->resolve) {
			this->Exists();
		}

		return *this;
	}

	const FixedString<Capacity>::data_type& GetFilename() const {
		return this->filename.data();
	}

	BSurface* GetSurface(BytePalette* pPalette = nullptr) const {
		return this->Exists() ? PCX::Instance->GetSurface(this->filename, pPalette) : nullptr;
	}

	bool Exists() const {
		if(!this->checked) {
			this->checked = true;
			if(this->filename) {
				auto pPCX = PCX::Instance;
				this->exists = (pPCX->GetSurface(this->filename) || pPCX->LoadFile(this->filename));
			}
		}
		return this->exists;
	}

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		char buffer[Capacity];
		if(pINI->ReadString(pSection, pKey, pDefault, buffer)) {
			*this = buffer;

			if(this->checked && !this->exists) {
				Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
			}
		}
		return buffer[0] != 0;
	}

	bool Load(AresStreamReader &Stm, bool RegisterForChange) {
		this->filename = nullptr;
		if(Stm.Load(*this)) {
			if(this->checked && this->exists) {
				this->checked = false;
				if(!this->Exists()) {
					Debug::Log(Debug::Severity::Warning, "PCX file '%s' was not found.\n", this->filename.data());
					Debug::RegisterParserError();
				}
			}
			return true;
		}
		return false;
	}

	bool Save(AresStreamWriter &Stm) const {
		Stm.Save(*this);
		return true;
	}

private:
	FixedString<Capacity> filename;
	bool resolve;
	mutable bool checked;
	mutable bool exists;
};

// provides storage for a csf label with automatic lookup.
class CSFText {
public:
	CSFText() noexcept {}
	explicit CSFText(nullptr_t) noexcept {}

	explicit CSFText(const char* label) noexcept {
		*this = label;
	}

	CSFText& operator = (CSFText const& rhs) = default;

	const CSFText& operator = (const char* label) {
		if(this->Label != label) {
			this->Label = label;
			this->Text = nullptr;

			if(this->Label) {
				this->Text = StringTable::LoadString(this->Label);
			}
		}

		return *this;
	}

	operator const wchar_t* () const {
		return this->Text;
	}

	bool empty() const {
		return !this->Text || !*this->Text;
	}

	bool load(AresStreamReader &Stm, bool RegisterForChange) {
		this->Text = nullptr;
		if(Stm.Load(this->Label.data())) {
			if(this->Label) {
				this->Text = StringTable::LoadString(this->Label);
			}
			return true;
		}
		return false;
	}

	bool save(AresStreamWriter &Stm) const {
		Stm.Save(this->Label.data());
		return true;
	}

	FixedString<0x20> Label;
	const wchar_t* Text{ nullptr };
};

// fixed string with read method
template <size_t Capacity>
class AresFixedString : public FixedString<Capacity> {
public:
	AresFixedString() = default;
	explicit AresFixedString(nullptr_t) noexcept {};
	explicit AresFixedString(const char* value) noexcept : FixedString(value) {}

	using FixedString::operator=;

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		if(pINI->ReadString(pSection, pKey, pDefault, Ares::readBuffer, FixedString::Size)) {
			if(!INIClass::IsBlank(Ares::readBuffer)) {
				*this = Ares::readBuffer;
			} else {
				*this = nullptr;
			}
		}
		return Ares::readBuffer[0] != 0;
	}
};

// a wrapper for an optional value
template <typename T, bool Persistable = false>
struct OptionalStruct {
	OptionalStruct() = default;
	explicit OptionalStruct(T value) noexcept : Value(std::move(value)), HasValue(true) {}

	OptionalStruct& operator= (T value) {
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	operator T& () noexcept {
		return this->Value;
	}

	operator const T& () const noexcept {
		return this->Value;
	}

	void clear() {
		this->Value = T();
		this->HasValue = false;
	}

	bool empty() const {
		return !this->HasValue;
	}

	const T& get() const noexcept {
		return this->Value;
	}

	bool load(AresStreamReader &Stm, bool RegisterForChange) {
		this->clear();

		return load(Stm, RegisterForChange, std::bool_constant<Persistable>());
	}

	bool save(AresStreamWriter &Stm) const {
		return save(Stm, std::bool_constant<Persistable>());
	}

private:
	bool load(AresStreamReader &Stm, bool RegisterForChange, std::true_type) {
		if(Stm.Load(this->HasValue)) {
			if(!this->HasValue || Savegame::ReadAresStream(Stm, this->Value, RegisterForChange)) {
				return true;
			}
		}
		return false;
	}

	bool load(AresStreamReader &Stm, bool RegisterForChangestd, std::false_type) {
		return true;
	}

	bool save(AresStreamWriter &Stm, std::true_type) const {
		Stm.Save(this->HasValue);
		if(this->HasValue) {
			Savegame::WriteAresStream(Stm, this->Value);
		}
		return true;
	}

	bool save(AresStreamWriter &Stm, std::false_type) const {
		return true;
	}

	T Value{};
	bool HasValue{ false };
};

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle {
	constexpr Handle() noexcept = default;

	constexpr explicit Handle(T value) noexcept
		: Value(value)
	{ }

	Handle(const Handle&) = delete;

	constexpr Handle(Handle&& other) noexcept
		: Value(other.release())
	{ }

	~Handle() noexcept {
		if(this->Value != Default) {
			Deleter{}(this->Value);
		}
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept {
		this->reset(other.release());
		return *this;
	}

	constexpr explicit operator bool() const noexcept {
		return this->Value != Default;
	}

	constexpr operator T () const noexcept {
		return this->Value;
	}

	constexpr T get() const noexcept {
		return this->Value;
	}

	T release() noexcept {
		return std::exchange(this->Value, Default);
	}

	void reset(T value) noexcept {
		Handle(this->Value);
		this->Value = value;
	}

	void clear() noexcept {
		Handle(std::move(*this));
	}

	bool load(AresStreamReader &Stm, bool RegisterForChange) {
		return Savegame::ReadAresStream(Stm, this->Value, RegisterForChange);
	}

	bool save(AresStreamWriter &Stm) const {
		return Savegame::WriteAresStream(Stm, this->Value);
	}

private:
	T Value{ Default };
};

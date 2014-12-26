#ifndef CONSTRUCTS_H_
#define CONSTRUCTS_H_

// custom paletted cameos
// TODO: add a static vector to buffer instances of the same Palette file?
#include <ConvertClass.h>
#include <ScenarioClass.h>
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

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

struct Leptons {
	explicit Leptons(int value = 0) : value(value) {}

	operator int() const {
		return this->value;
	}

	int value;
};

class CustomPalette {
public:
	class PaletteMode {
	public:
		typedef int Value;
		enum {
			Default = 0,
			Temperate = 1
		};
	};

	PaletteMode::Value Mode;
	UniqueGamePtr<ConvertClass> Convert;
	UniqueGamePtr<BytePalette> Palette;

	CustomPalette(PaletteMode::Value mode = PaletteMode::Default) :
		Mode(mode),
		Convert(nullptr),
		Palette(nullptr)
	{};

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	bool LoadFromINI(CCINIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		if(pINI->ReadString(pSection, pKey, pDefault, Ares::readBuffer, Ares::readLength)) {
			if(char* suffix = strstr(Ares::readBuffer, "~~~")) {
				const char* theaterSpecific = Theater::GetTheater(ScenarioClass::Instance->Theater).Extension;
				suffix[0] = theaterSpecific[0];
				suffix[1] = theaterSpecific[1];
				suffix[2] = theaterSpecific[2];
			}

			this->Clear();

			if(auto pPal = FileSystem::AllocatePalette(Ares::readBuffer)) {
				this->Palette.reset(pPal);
				this->CreateConvert();
			}

			return this->Convert != nullptr;
		}
		return false;
	};

	bool Load(AresStreamReader &Stm, bool RegisterForChange) {
		this->Clear();

		bool hasPalette = false;
		auto ret = Stm.Load(this->Mode) && Stm.Load(hasPalette);

		if(ret && hasPalette) {
			this->Palette.reset(GameCreate<BytePalette>());
			ret = Stm.Load(*this->Palette);

			if(ret) {
				this->CreateConvert();
			}
		}

		return ret;
	}

	bool Save(AresStreamWriter &Stm) const {
		Stm.Save(this->Mode);
		Stm.Save(this->Palette != nullptr);
		if(this->Palette) {
			Stm.Save(*this->Palette);
		}
		return true;
	}

private:
	void Clear() {
		this->Convert = nullptr;
		this->Palette = nullptr;
	}

	void CreateConvert() {
		ConvertClass* buffer = nullptr;
		if(this->Mode == PaletteMode::Temperate) {
			buffer = GameCreate<ConvertClass>(this->Palette.get(), FileSystem::TEMPERAT_PAL, DSurface::Primary, 53, false);
		} else {
			buffer = GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate, 1, false);
		}
		this->Convert.reset(buffer);
	}
};

ENABLE_ARES_PERSISTENCE(CustomPalette);

// vector of char* with builtin storage
template<class T>
class VectorNames {
protected:
	DynamicVectorClass<char *> Strings;
	char * Buffer;

public:
	char* operator [](int Index) {
		if(Index < 0 || Index > this->Strings.Count) {
			return nullptr;
		}
		return this->Strings.GetItem(Index);
	}

	T * FindItem(int Index) {
		return T::Find((*this)[Index]);
	}

	const DynamicVectorClass<char *> & Entries() const{
		return this->Strings;
	}

	char ** ToString() {
		return this->Strings.Items;
	}

	int Count() const {
		return this->Strings.Count;
	}

	VectorNames<T>(const char * Buf = nullptr) {
		this->Buffer = _strdup(Buf);
	}

	void Tokenize(const char * Buf = nullptr) {
		if(Buf) {
			if(this->Buffer) {
				free(this->Buffer);
			}
			this->Buffer = _strdup(Buf);
		}
		this->Strings.Clear();

		char* context = nullptr;
		for(char * cur = strtok_s(this->Buffer, ",", &context); cur && *cur; cur = strtok_s(nullptr, ",", &context)) {
			this->Strings.AddItem(cur);
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
		this->values.emplace_back(key, TValue());
		return this->values.back().second;
	}

	TValue* find(const TKey& key) {
		if(auto pValue = static_cast<const AresMap*>(this)->find(key)) {
			return const_cast<TValue*>(pValue);
		}
		return nullptr;
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

	void erase(const TKey& key) {
		auto it = this->get_iterator(key);
		if(it != this->values.end()) {
			this->values.erase(it);
		}
	}

	bool contains(const TKey& key) const {
		return this->get_iterator(key) != this->values.end();
	}

	void insert(const TKey& key, TValue value) {
		(*this)[key] = value;
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

private:
	using container_t = std::vector<std::pair<TKey, TValue>>;

	typename container_t::const_iterator get_iterator(const TKey& key) const {
		return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item) {
			return item.first == key;
		});
	}

	container_t values;
};

// pcx filename storage with optional automatic loading
class AresPCXFile {
	static const size_t Capacity = 0x20;
public:
	explicit AresPCXFile(bool autoResolve = true) : filename(), resolve(autoResolve), checked(false), exists(false) {
	}

	AresPCXFile(const char* filename, bool autoResolve = true) : AresPCXFile(autoResolve) {
		*this = filename;
	}

	AresPCXFile& operator = (const char* filename) {
		this->filename = filename;
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
		if(pINI->ReadString(pSection, pKey, pDefault, buffer, Capacity)) {
			*this = buffer;

			if(this->checked && !this->exists) {
				Debug::INIParseFailed(pSection, pKey, this->filename, "PCX file not found.");
			}
		}
		return buffer[0] != 0;
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
	CSFText(const char* label = nullptr) : Text(nullptr) {
		*this = label;
	}

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

	const CSFText& operator = (const CSFText& other) {
		if(this != &other) {
			this->Label = other.Label;
			this->Text = other.Text;
		}

		return *this;
	}

	operator const wchar_t* () const {
		return this->Text;
	}

	bool empty() const {
		return !this->Text || !*this->Text;
	}

	FixedString<0x20> Label;
	const wchar_t* Text;
};

// fixed string with read method
template <size_t Capacity>
class AresFixedString : public FixedString<Capacity> {
public:
	explicit AresFixedString(const char* value = nullptr) : FixedString(value) {
	}

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
template <typename T>
struct OptionalStruct {
	OptionalStruct() : Value(T()), HasValue(false) {}
	explicit OptionalStruct(T value) : Value(value), HasValue(true) {}

	OptionalStruct& operator= (T value) {
		this->Value = value;
		this->HasValue = true;
		return *this;
	}

	operator T& () {
		return this->Value;
	}

	operator const T& () const {
		return this->Value;
	}

	void clear() {
		this->Value = T();
		this->HasValue = false;
	}

	bool empty() const {
		return !this->HasValue;
	}

	const T& get() const {
		return this->Value;
	}

private:
	T Value;
	bool HasValue;
};

#endif

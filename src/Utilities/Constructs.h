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

#include <cstring>
#include <memory>

#include "../Ares.h"
#include "../Ares.CRT.h"

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

	~CustomPalette() {
		this->Clear();
	}

	ConvertClass* GetConvert() const {
		return this->Convert.get();
	}

	bool LoadFromINI(CCINIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "") {
		if(pINI->ReadString(pSection, pKey, pDefault, Ares::readBuffer, Ares::readLength)) {
			if(char* suffix = strstr(Ares::readBuffer, "~~~")) {
				const char* theaterSpecific = Theater::Array[ScenarioClass::Instance->Theater].Extension;
				suffix[0] = theaterSpecific[0];
				suffix[1] = theaterSpecific[1];
				suffix[2] = theaterSpecific[2];
			}

			this->Clear();

			this->Palette = this->ReadPalette(Ares::readBuffer);
			if(this->Palette) {
				this->CreateConvert();
			}

			return this->Convert != nullptr;
		}
		return false;
	};

private:
	void Clear() {
		this->Convert = nullptr;
		this->Palette = nullptr;
	}

	UniqueGamePtr<BytePalette> ReadPalette(const char* filename) {
		UniqueGamePtr<BytePalette> ret = nullptr;

		CCFileClass file(filename);
		if(auto pData = file.ReadWholeFile()) {
			auto pPal = reinterpret_cast<BytePalette*>(pData);

			BytePalette* buffer = GameCreate<BytePalette>();
			ret = UniqueGamePtr<BytePalette>(buffer);

			// convert 6 bits to 8 bits. not correct,
			// but this is what the game does
			for(int i = 0; i < 256; ++i) {
				ret->Entries[i].R = pPal->Entries[i].R << 2;
				ret->Entries[i].G = pPal->Entries[i].G << 2;
				ret->Entries[i].B = pPal->Entries[i].B << 2;
			}

			GameDelete(pData);
		}

		return ret;
	}

	void CreateConvert() {
		ConvertClass* buffer = nullptr;
		if(this->Mode == PaletteMode::Temperate) {
			auto pTargetPal = (BytePalette*)0x885780; // pointer to TEMPERAT_PAL (not the Convert!)
			buffer = GameCreate<ConvertClass>(this->Palette.get(), pTargetPal, DSurface::Primary, 53, false);
		} else {
			buffer = GameCreate<ConvertClass>(this->Palette.get(), this->Palette.get(), DSurface::Alternate, 1, false);
		}
		this->Convert = UniqueGamePtr<ConvertClass>(buffer);
	}
};

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

// provides storage for a csf label with automatic lookup.
class CSFText {
public:
	CSFText(const char* label = nullptr) : Text(nullptr) {
		*this = label;
	}

	const CSFText& operator = (const char* label) {
		if(this->Label != label) {
			this->Label[0] = 0;
			this->Text = nullptr;

			if(label && *label) {
				this->Label = label;
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
	AresFixedString(const char* value = nullptr) : FixedString(value) {
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

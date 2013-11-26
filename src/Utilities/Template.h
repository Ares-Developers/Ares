#ifndef ARES_TEMPLATE_H
#define ARES_TEMPLATE_H

#include <stdexcept>
#include <cstring>

#include <MouseClass.h>
#include <TechnoClass.h>
#include <Helpers/Type.h>
#include "INIParser.h"
#include "Enums.h"
#include "Constructs.h"
#include "Iterator.h"

/**
 * More fancy templates!
 * This one is just a nicer-looking INI Parser... the fun starts with the next one
 */

template<typename T>
class Valueable {
protected:
	T    Value;
public:
	typedef T MyType;
	typedef typename CompoundT<T>::BaseT MyBase;
	Valueable(T Default = T()) : Value(Default) {};

	virtual ~Valueable() {}

	operator const T& () const {
		return this->Get();
	}

	// only allow this when explict works, otherwise
	// the always-non-null pointer will be used in conditionals.
	//explicit operator T* () {
	//	return this->GetEx();
	//}

	T operator -> () const {
		return this->Get();
	}

	T* operator & () {
		return this->GetEx();
	}

	bool operator ! () const {
		return this->Get() == 0;
	};

	virtual const T& Get() const {
		return this->Value;
	}

	virtual T * GetEx() {
		return &this->Value;
	}

	virtual const T * GetEx() const {
		return &this->Value;
	}

	virtual void Set(const T& val) {
		this->Value = val;
	}

	virtual void SetEx(T* val) {
		this->Value = *val;
	}

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		ImplementThisFunction();
	};

	void Parse(INI_EX *parser, const char* pSection, const char* pKey, bool Allocate = 0) {
		if(parser->ReadString(pSection, pKey)) {
			const char * val = parser->value();
			if(auto parsed = (Allocate ? MyBase::FindOrAllocate : MyBase::Find)(val)) {
				this->Set(parsed);
			} else {
				Debug::INIParseFailed(pSection, pKey, val);
			}
		}
	}
};

// more fun
template<typename Lookuper>
class ValueableIdx : public Valueable<int> {
public:
	ValueableIdx(int Default) : Valueable<int>(Default) {};

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			const char * val = parser->value();
			int idx = Lookuper::FindIndex(val);
			if(idx != -1 || INIClass::IsBlank(val)) {
				this->Set(idx);
			} else {
				Debug::INIParseFailed(pSection, pKey, val);
			}
		}
	}
};

template<typename T>
class Nullable : public Valueable<T> {
protected:
	bool HasValue;
public:
	Nullable(): Valueable<T>(T()), HasValue(false) {};
	Nullable(T Val): Valueable<T>(Val), HasValue(true) {};

	bool isset() const {
		return this->HasValue;
	}

	using Valueable<T>::Get;

	const T& Get(const T& defVal) const {
		return this->isset() ? Valueable<T>::Get() : defVal;
	}

	using Valueable<T>::GetEx;

	T* GetEx(T* defVal) {
		return this->isset() ? Valueable<T>::GetEx() : defVal;
	}

	const T* GetEx(const T* defVal) const {
		return this->isset() ? Valueable<T>::GetEx() : defVal;
	}

	virtual void Set(const T& val) {
		Valueable<T>::Set(val);
		this->HasValue = true;
	}

	virtual void SetEx(T* val) {
		Valueable<T>::SetEx(val);
		this->HasValue = true;
	}

	void Reset() {
		Valueable<T>::Set(T());
		this->HasValue = false;
	}
};

template<typename Lookuper>
class NullableIdx : public Nullable<int> {
public:
	NullableIdx() : Nullable<int>() {};
	NullableIdx(int Val) : Nullable<int>(Val) {};

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			const char * val = parser->value();
			int idx = Lookuper::FindIndex(val);
			if(idx != -1 || INIClass::IsBlank(val)) {
				this->Set(idx);
			} else {
				Debug::INIParseFailed(pSection, pKey, val);
			}
		}
	}
};

/*
 * This one is for data that defaults to some original flag value but can be overwritten with custom values
 * Bind() it to a data address from where to take the value
 * (e.g. &RulesClass::Global()->RadBeamColor for custom-colorizable rad waves)
 * and Set() it to a fixed value
 */

template<typename T>
class Customizable : public Valueable<T> {
	bool Customized;
	T*   Default;
public:
	Customizable(T* alias = nullptr) : Valueable<T>(T()), Customized(false), Default(alias) {};

	void Bind(T* to) {
		if(!this->Customized) {
			this->Default = to;
		}
	}

	void BindEx(const T& to) {
		if(!this->Customized) {
			this->Value = to;
			this->Default = &this->Value;
		}
	}

	virtual const T& Get() const {
		return (!this->Customized && this->Default)
		 ? *this->Default
		 : this->Value;
		;
	}

	virtual void Set(const T& val) {
		this->Customized = true;
		this->Value = val;
	}

	virtual T* GetEx() {
		return this->Customized
		 ? &this->Value
		 : this->Default
		;
	}

	virtual void SetEx(T* val) {
		this->Customized = true;
		this->Value = *val;
	}

	void Lock() {
		if(!this->Customized) {
			if(this->Default) {
				this->Value = *this->Default;
			}
			this->Customized = true;
		}
	}
};

/*
 * This template is for something that varies depending on a unit's Veterancy Level
 * Promotable<int> PilotChance; // class def
 * PilotChance(); // ctor init-list
 * PilotChance->Read(..., "Base%s"); // load from ini
 * PilotChance->Get(Unit); // usage
 */
template<typename T>
class Promotable {
public:
	T Rookie;
	T Veteran;
	T Elite;

	void SetAll(const T& val) {
		this->Elite = this->Veteran = this->Rookie = val;
	}

	void Read(CCINIClass *pINI, const char *Section, const char *BaseFlag) {
		unsigned int buflen = strlen(BaseFlag) + 8;
		char *FlagName = new char[buflen];

		Customizable<T> Placeholder;
		INI_EX exINI(pINI);
		Placeholder.Set(this->Rookie);

		_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Rookie");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Rookie = Placeholder.Get();

		Placeholder.Set(this->Veteran);
		_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Veteran");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Veteran = Placeholder.Get();

		Placeholder.Set(this->Elite);
		_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Elite");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Elite = Placeholder.Get();

		delete[] FlagName;
	}

	const T* GetEx(TechnoClass* pTechno) const {
		return &this->Get(pTechno);
	}

	const T& Get(TechnoClass* pTechno) const {
		VeterancyStruct *XP = &pTechno->Veterancy;
		if(XP->IsElite()) {
			return this->Elite;
		}
		if(XP->IsVeteran()) {
			return this->Veteran;
		}
		return this->Rookie;
	}
};


// specializations

template<>
void Valueable<bool>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	bool buffer = this->Get();
	if(parser->ReadBool(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");
	}
};

template<>
void Valueable<int>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	int buffer = this->Get();
	if(parser->ReadInteger(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid number");
	}
};

template<>
void Valueable<BYTE>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	int buffer = this->Get();
	if(parser->ReadInteger(pSection, pKey, &buffer)) {
		if(buffer <= 255 && buffer >= 0) {
			const BYTE result((BYTE)buffer); // shut up shut up shut up C4244
			this->Set(result);
		} else {
			Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid number between 0 and 255 inclusive.");
		}
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid number");
	}
};

template<>
void Valueable<float>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	double buffer = this->Get();
	if(parser->ReadDouble(pSection, pKey, &buffer)) {
		this->Set(static_cast<float>(buffer));
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid floating point number");
	}
};

template<>
void Valueable<double>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	double buffer = this->Get();
	if(parser->ReadDouble(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid floating point number");
	}
};

template<>
void Valueable<ColorStruct>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	ColorStruct buffer = this->Get();
	if(parser->Read3Bytes(pSection, pKey, (byte*)&buffer)) {
		this->Set(buffer);
	} else if(parser->declared()) {
		Debug::INIParseFailed(pSection, pKey, parser->value(), "Expected a valid R,G,B color");
	}
};

template<>
void Valueable<CSFText>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	if(parser->ReadString(pSection, pKey)) {
		this->Set(parser->value());
	}
};

template<>
void Valueable<SHPStruct *>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	if(parser->ReadString(pSection, pKey)) {
		char flag[256];
		const char * val = parser->value();
		_snprintf_s(flag, 255, "%s.shp", val);
		if(SHPStruct *image = FileSystem::LoadSHPFile(flag)) {
			this->Set(image);
		} else {
			Debug::DevLog(Debug::Warning, "Failed to find file %s referenced by [%s]%s=%s", flag, pSection, pKey, val);
		}
	}
};

template<>
void Valueable<MouseCursor>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	Customizable<int> Placeholder;

	MouseCursor *Cursor = this->GetEx();

	char pFlagName [32];
	_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
	Placeholder.Set(Cursor->Frame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Frame = Placeholder.Get();

	_snprintf_s(pFlagName, 31, "%s.Count", pKey);
	Placeholder.Set(Cursor->Count);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Count = Placeholder.Get();

	_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
	Placeholder.Set(Cursor->Interval);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Interval = Placeholder.Get();

	_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
	Placeholder.Set(Cursor->MiniFrame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniFrame = Placeholder.Get();

	_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
	Placeholder.Set(Cursor->MiniCount);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniCount = Placeholder.Get();

	_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);
	if(parser->ReadString(pSection, pFlagName)) {
		char *buffer = const_cast<char *>(parser->value());
		char *context = nullptr;
		char *hotx = strtok_s(buffer, ",", &context);
		if(!strcmp(hotx, "Left")) this->Value.HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) this->Value.HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) this->Value.HotX = hotspx_right;

		if(char *hoty = strtok_s(nullptr, ",", &context)) {
			if(!strcmp(hoty, "Top")) this->Value.HotY = hotspy_top;
			else if(!strcmp(hoty, "Middle")) this->Value.HotY = hotspy_middle;
			else if(!strcmp(hoty, "Bottom")) this->Value.HotY = hotspy_bottom;
		}
	}
};

template<>
void Valueable<RocketStruct>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	Customizable<bool> BoolPlaceholder;
	Customizable<int> IntPlaceholder;
	Customizable<float> FloatPlaceholder;
	Customizable<AircraftTypeClass*> TypePlaceholder;

	RocketStruct* rocket = this->GetEx();

	char pFlagName[0x40];
	_snprintf_s(pFlagName, 0x3F, "%s.PauseFrames", pKey);
	IntPlaceholder.Set(rocket->PauseFrames);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PauseFrames = IntPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.TiltFrames", pKey);
	IntPlaceholder.Set(rocket->TiltFrames);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->TiltFrames = IntPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.PitchInitial", pKey);
	FloatPlaceholder.Set(rocket->PitchInitial);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PitchInitial = FloatPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.PitchFinal", pKey);
	FloatPlaceholder.Set(rocket->PitchFinal);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PitchFinal = FloatPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.TurnRate", pKey);
	FloatPlaceholder.Set(rocket->TurnRate);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->TurnRate = FloatPlaceholder.Get();

	// sic! integer read like a float.
	_snprintf_s(pFlagName, 0x3F, "%s.RaiseRate", pKey);
	FloatPlaceholder.Set(static_cast<float>(rocket->RaiseRate));
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->RaiseRate = Game::F2I(FloatPlaceholder.Get());

	_snprintf_s(pFlagName, 0x3F, "%s.Acceleration", pKey);
	FloatPlaceholder.Set(rocket->Acceleration);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Acceleration = FloatPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.Altitude", pKey);
	IntPlaceholder.Set(rocket->Altitude);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Altitude = IntPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.Damage", pKey);
	IntPlaceholder.Set(rocket->Damage);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Damage = IntPlaceholder.Get();
	
	_snprintf_s(pFlagName, 0x3F, "%s.EliteDamage", pKey);
	IntPlaceholder.Set(rocket->EliteDamage);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->EliteDamage = IntPlaceholder.Get();
	
	_snprintf_s(pFlagName, 0x3F, "%s.BodyLength", pKey);
	IntPlaceholder.Set(rocket->BodyLength);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->BodyLength = IntPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.LazyCurve", pKey);
	BoolPlaceholder.Set(rocket->LazyCurve);
	BoolPlaceholder.Read(parser, pSection, pFlagName);
	rocket->LazyCurve = BoolPlaceholder.Get();

	_snprintf_s(pFlagName, 0x3F, "%s.Type", pKey);
	TypePlaceholder.Set(rocket->Type);
	TypePlaceholder.Parse(parser, pSection, pFlagName);
	rocket->Type = TypePlaceholder.Get();
};

template<class T>
class ValueableVector : public std::vector<T> {
protected:
	bool _Defined;
public:
	typedef T MyType;
	typedef typename CompoundT<T>::BaseT MyBase;

	ValueableVector() : std::vector<T>(), _Defined(false) {};

	virtual ~ValueableVector() {}

	virtual void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			this->clear();
			this->_Defined = true;
			this->Split(parser, pSection, pKey, Ares::readBuffer);
		}
	}

	bool Contains(const T &other) const {
		return std::find(this->begin(), this->end(), other) != this->end();
	}

	int IndexOf(const T &other) const {
		auto it = std::find(this->begin(), this->end(), other);
		if(it != this->end()) {
			return it - this->begin();
		}
		return -1;
	}

	bool Defined() const {
		return this->_Defined;
	}

	virtual Iterator<T> GetElements() const {
		return Iterator<T>(*this);
	}

protected:
	virtual void Split(INI_EX *parser, const char* pSection, const char* pKey, char* pValue) {
		// if we were able to get the flag in question, take it apart and check the tokens...
		char* context = nullptr;
		for(char *cur = strtok_s(pValue, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			Parse(parser, pSection, pKey, cur);
		}
	}

	void Parse(INI_EX *parser, const char* pSection, const char* pKey, char* pValue) {
		T buffer = T();
		if(Parser<T>::Parse(pValue, &buffer)) {
			this->push_back(buffer);
		} else if(!INIClass::IsBlank(pValue)) {
			Debug::INIParseFailed(pSection, pKey, pValue);
		}
	}
};

template<>
void ValueableVector<TechnoTypeClass *>::Parse(INI_EX *parser, const char* pSection, const char* pKey, char* pValue) {
	// ...against the various object types; if we find one, place it in the value list
	if(auto pAircraftType = AircraftTypeClass::Find(pValue)) {
		this->push_back(pAircraftType);
	} else if(auto pBuildingType = BuildingTypeClass::Find(pValue)) {
		this->push_back(pBuildingType);
	} else if(auto pInfantryType = InfantryTypeClass::Find(pValue)) {
		this->push_back(pInfantryType);
	} else if(auto pUnitType = UnitTypeClass::Find(pValue)) {
		this->push_back(pUnitType);
	} else if(!INIClass::IsBlank(pValue)) {
		Debug::INIParseFailed(pSection, pKey, pValue);
	}
}

template<class T>
class NullableVector : public ValueableVector<T> {
protected:
	bool _HasValue;
public:
	NullableVector() : ValueableVector<T>(), _HasValue(false) {};

	virtual void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			this->clear();
			this->_Defined = true;

			// provide a way to reset to default
			if(!_strcmpi(Ares::readBuffer, "<default>")) {
				this->_HasValue = false;
			} else {
				this->_HasValue = true;
				this->Split(parser, pSection, pKey, Ares::readBuffer);
			}
		}
	}

	bool HasValue() const {
		return this->_HasValue;
	}

	using ValueableVector<T>::GetElements;

	virtual Iterator<T> GetElements(Iterator<T> defElements) const {
		if(!this->_HasValue) {
			return defElements;
		}

		return ValueableVector<T>::GetElements();
	}
};

template<typename Lookuper>
class ValueableIdxVector : public ValueableVector<int> {
protected:
	virtual void Split(INI_EX *parser, const char* pSection, const char* pKey, char* pValue) {
		// split the string and look up the tokens. only valid tokens are added.
		char* context = nullptr;
		for(char* cur = strtok_s(pValue, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			int idx = Lookuper::FindIndex(cur);
			if(idx != -1) {
				this->push_back(idx);
			} else if(!INIClass::IsBlank(cur)) {
				Debug::INIParseFailed(pSection, pKey, cur);
			}
		}
	}
};

template<typename Lookuper>
class NullableIdxVector : public NullableVector<int> {
protected:
	virtual void Split(INI_EX *parser, const char* pSection, const char* pKey, char* pValue) {
		// split the string and look up the tokens. only valid tokens are added.
		char* context = nullptr;
		for(char* cur = strtok_s(pValue, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			int idx = Lookuper::FindIndex(cur);
			if(idx != -1) {
				this->push_back(idx);
			} else if(!INIClass::IsBlank(cur)) {
				Debug::INIParseFailed(pSection, pKey, cur);
			}
		}
	}
};

template<typename T>
class ValueableEnum : public Valueable<typename T::Value> {
public:
	typedef typename T::Value ValueType;

	ValueableEnum(ValueType Default = ValueType()) : Valueable<ValueType>(Default) {};

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			ValueType buffer = this->Get();
			if(T::Parse(Ares::readBuffer, &buffer)) {
				this->Set(buffer);
			} else if(!INIClass::IsBlank(Ares::readBuffer)) {
				Debug::INIParseFailed(pSection, pKey, Ares::readBuffer);
			}
		}
	};
};


//template class Valueable<bool>;
//template class Valueable<int>;
//template class Valueable<double>;
//template class Valueable<ColorStruct>;
//template class Valueable<SHPStruct *>;
//template class Valueable<MouseCursor>;

#endif

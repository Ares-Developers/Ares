#ifndef ARES_TEMPLATE_H
#define ARES_TEMPLATE_H

#include <stdexcept>

#include <MouseClass.h>
#include <TechnoClass.h>
#include <Helpers/Type.h>
#include "INIParser.h"

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

	operator T () {
		return this->Get();
	}

	operator T* () {
		return this->GetEx();
	}

	T* operator & () {
		return this->GetEx();
	}

	bool operator != (T other) const {
		return this->Value != other;
	};

	bool operator ! () {
		return this->Get() == 0;
	};

	virtual T Get() {
		return this->Value;
	}

	virtual T* GetEx() {
		return &this->Value;
	}

	virtual void Set(T val) {
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
			this->Set((Allocate ? MyBase::FindOrAllocate : MyBase::Find)(parser->value()));
		}
	}
};

// more fun
template<typename T, typename Lookuper>
class ValueableIdx {
protected:
	T    Value;
public:
	ValueableIdx(T Default) : Value(Default) {};

	operator T () {
		return this->Get();
	}

	operator T* () {
		return this->GetEx();
	}

	T* operator & () {
		return this->GetEx();
	}

	virtual T Get() {
		return this->Value;
	}

	virtual void Set(T val) {
		this->Value = val;
	}

	virtual T* GetEx() {
		return &this->Value;
	}

	virtual void SetEx(T* val) {
		this->Value = *val;
	}

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
		if(parser->ReadString(pSection, pKey)) {
			int idx = Lookuper::FindIndex(parser->value());
			if(idx != -1) {
				this->Set(idx);
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
	Customizable(T* alias = NULL) : Valueable<T>(T()), Customized(false), Default(alias) {};

	void Bind(T* to) {
		if(!this->Customized) {
			this->Default = to;
		}
	}

	virtual T Get() {
		return this->Customized
		 ? this->Value
		 : this->Default ? *this->Default : T()
		;
	}

	virtual void Set(T val) {
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
 * PilotChance(NULL); // ctor init-list
 * PilotChance->BindTo(Unit); // instantiation
 * PilotChance->Get(); // usage
 */
template<typename T>
class Promotable {
	TechnoClass * _BindTo;
public:
	T Rookie;
	T Veteran;
	T Elite;

	Promotable(TechnoClass * Object = NULL) : _BindTo(Object) {};
	Promotable<T>* BindTo(TechnoClass * Object) {
		this->_BindTo = Object;
		return this;
	}

	void SetAll(T val) {
		this->Elite = this->Veteran = this->Rookie = val;
	}

	void LoadFromINI(CCINIClass *pINI, const char *Section, const char *BaseFlag) {
		unsigned int buflen = strlen(BaseFlag) + 8;
		char *FlagName = new char[buflen];

		Customizable<T> Placeholder;
		INI_EX exINI(pINI);
		Placeholder.Set(this->Rookie);

		_snprintf(FlagName, buflen, BaseFlag, "Rookie");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Rookie = Placeholder.Get();

		Placeholder.Set(this->Veteran);
		_snprintf(FlagName, buflen, BaseFlag, "Veteran");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Veteran = Placeholder.Get();

		Placeholder.Set(this->Elite);
		_snprintf(FlagName, buflen, BaseFlag, "Elite");
		Placeholder.Read(&exINI, Section, FlagName);
		this->Elite = Placeholder.Get();

		delete[] FlagName;
	}

	T* GetEx() {
		if(!this->_BindTo) {
			Debug::Log("Promotable<T> invoked without an owner!\n");
			throw std::logic_error("Promotable<T> invoked without an owner!\n");
		}
		VeterancyStruct *XP = &this->_BindTo->Veterancy;
		if(XP->IsElite()) {
			return &this->Elite;
		}
		if(XP->IsVeteran()) {
			return &this->Veteran;
		}
		return &this->Rookie;
	}

	T Get() {
		return *this->GetEx();
	}
};


// specializations

template<>
void Valueable<bool>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	bool buffer = this->Get();
	if(parser->ReadBool(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
};

template<>
void Valueable<int>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	int buffer = this->Get();
	if(parser->ReadInteger(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
};

template<>
void Valueable<float>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	double buffer = this->Get();
	if(parser->ReadDouble(pSection, pKey, &buffer)) {
		this->Set(static_cast<float>(buffer));
	}
};

template<>
void Valueable<double>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	double buffer = this->Get();
	if(parser->ReadDouble(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
};

template<>
void Valueable<ColorStruct>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	ColorStruct buffer = this->Get();
	if(parser->Read3Bytes(pSection, pKey, (byte*)&buffer)) {
		this->Set(buffer);
	}
};

template<>
void Valueable<SHPStruct *>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	if(parser->ReadString(pSection, pKey)) {
		char flag[256];
		_snprintf(flag, 256, "%s.shp", parser->value());
		SHPStruct *image = FileSystem::LoadSHPFile(flag);
		if(image) {
			this->Set(image);
		}
	}
};

template<>
void Valueable<MouseCursor>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	Customizable<int> Placeholder;

	MouseCursor *Cursor = this->GetEx();

	char pFlagName [32];
	_snprintf(pFlagName, 32, "%s.Frame", pKey);
	Placeholder.Set(Cursor->Frame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Frame = Placeholder.Get();

	_snprintf(pFlagName, 32, "%s.Count", pKey);
	Placeholder.Set(Cursor->Count);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Count = Placeholder.Get();

	_snprintf(pFlagName, 32, "%s.Interval", pKey);
	Placeholder.Set(Cursor->Count);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Interval = Placeholder.Get();

	_snprintf(pFlagName, 32, "%s.MiniFrame", pKey);
	Placeholder.Set(Cursor->MiniFrame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniFrame = Placeholder.Get();

	_snprintf(pFlagName, 32, "%s.MiniCount", pKey);
	Placeholder.Set(Cursor->MiniCount);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniCount = Placeholder.Get();

	_snprintf(pFlagName, 32, "%s.HotSpot", pKey);
	if(parser->ReadString(pSection, pFlagName)) {
		char *buffer = const_cast<char *>(parser->value());
		char *hotx = strtok(buffer, ",");
		if(!strcmp(hotx, "Left")) this->Value.HotX = hotspx_left;
		else if(!strcmp(hotx, "Center")) this->Value.HotX = hotspx_center;
		else if(!strcmp(hotx, "Right")) this->Value.HotX = hotspx_right;

		char *hoty = strtok(NULL, ",");
		if(!strcmp(hoty, "Top")) this->Value.HotY = hotspy_top;
		else if(!strcmp(hoty, "Middle")) this->Value.HotY = hotspy_middle;
		else if(!strcmp(hoty, "Bottom")) this->Value.HotY = hotspy_bottom;
	}
};

//template class Valueable<bool>;
//template class Valueable<int>;
//template class Valueable<double>;
//template class Valueable<ColorStruct>;
//template class Valueable<SHPStruct *>;
//template class Valueable<MouseCursor>;

#endif

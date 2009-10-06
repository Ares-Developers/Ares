#ifndef ARES_TEMPLATE_H
#define ARES_TEMPLATE_H

#include <stdexcept>

#include <TechnoClass.h>
#include "INIParser.h"
#include "Type.h"

/**
 * More fancy templates!
 * This one is for data that defaults to some original flag value but can be overwritten with custom values
 * Bind() it to a data address from where to take the value 
 * (e.g. &RulesClass::Global()->RadBeamColor for custom-colorizable rad waves) 
 * and Set() it to a fixed value
 */

template<typename T>
class Customizable {
	bool Customized;
	T*   Default;
	T    Value;
public:
	typedef T MyType;
	typedef typename CompoundT<T>::BaseT MyBase;
	Customizable(T* alias = NULL) : Customized(false), Default(alias) {};

	void Bind(T* to) {
		if(!this->Customized) {
			this->Default = to;
		}
	}

	void Set(T val) {
		this->Customized = true;
		this->Value = val;
	}

	void SetEx(T* val) {
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

	T Get() {
		return this->Customized
		 ? this->Value
		 : this->Default ? *this->Default : T()
		;
	}

	T* GetEx() {
		return this->Customized
		 ? &this->Value
		 : this->Default
		;
	}

	void Read(INI_EX *parser, const char* pSection, const char* pKey) {
	
	}

	void ReadFind(INI_EX *parser, const char* pSection, const char* pKey, bool Allocate = 0) {
//		T buffer = this->Get();
		if(parser->ReadString(pSection, pKey)) {
//			if(buffer = ) {
			this->Set((Allocate ? MyBase::FindOrAllocate : MyBase::Find)(parser->value()));
//			}
		}
	}
};

template<>
void Customizable<bool>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	bool buffer = this->Get();
	if(parser->ReadBool(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
};

template<>
void Customizable<int>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	int buffer = this->Get();
	if(parser->ReadInteger(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
}

template<>
void Customizable<double>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	double buffer = this->Get();
	if(parser->ReadDouble(pSection, pKey, &buffer)) {
		this->Set(buffer);
	}
}

template<>
void Customizable<ColorStruct>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	ColorStruct buffer = this->Get();
	if(parser->Read3Bytes(pSection, pKey, (byte*)&buffer)) {
		this->Set(buffer);
	}
}

template<>
void Customizable<SHPStruct *>::Read(INI_EX *parser, const char* pSection, const char* pKey) {
	if(parser->ReadString(pSection, pKey)) {
		char flag[256];
		_snprintf(flag, 256, "%s.shp", parser->value());
		SHPStruct *image = FileSystem::LoadSHPFile(flag);
		if(image) {
			this->Set(image);
		}
	}
}
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
		VeterancyStruct *XP = this->_BindTo->get_Veterancy();
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

#endif

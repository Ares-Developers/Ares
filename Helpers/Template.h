#ifndef ARES_TEMPLATE_H
#define ARES_TEMPLATE_H

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

/*
 */

#endif

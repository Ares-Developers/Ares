#ifndef ARES_TEMPLATE_H
#define ARES_TEMPLATE_H

#include "Iterator.h"

#include <MouseClass.h>
#include <FootClass.h>

class INI_EX;

/**
 * More fancy templates!
 * This one is just a nicer-looking INI Parser... the fun starts with the next one
 */

template<typename T>
class Valueable {
protected:
	T    Value;
public:
	typedef T value_type;
	typedef std::remove_pointer_t<T> base_type;
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

	inline void Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate = false);
};

// more fun
template<typename Lookuper>
class ValueableIdx : public Valueable<int> {
public:
	ValueableIdx() : Valueable<int>(-1) {};
	ValueableIdx(int Default) : Valueable<int>(Default) {};

	inline void Read(INI_EX &parser, const char* pSection, const char* pKey);
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

	T Get(const T& defVal) const {
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
	NullableIdx() : Nullable<int>(-1) { 
		this->HasValue = false;
	};

	NullableIdx(int Val) : Nullable<int>(Val) {};

	inline void Read(INI_EX &parser, const char* pSection, const char* pKey);
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

	inline void Read(CCINIClass *pINI, const char *Section, const char *BaseFlag);

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


template<class T>
class ValueableVector : public std::vector<T> {
protected:
	bool defined;
public:
	typedef T value_type;
	typedef std::remove_pointer_t<T> base_type;

	ValueableVector() : std::vector<T>(), defined(false) {};

	virtual ~ValueableVector() {}

	inline virtual void Read(INI_EX &parser, const char* pSection, const char* pKey);

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
		return this->defined;
	}

	virtual Iterator<T> GetElements() const {
		return Iterator<T>(*this);
	}

protected:
	inline virtual void Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue);

	inline void Parse(INI_EX &parser, const char* pSection, const char* pKey, char* pValue);
};

template<class T>
class NullableVector : public ValueableVector<T> {
protected:
	bool hasValue;
public:
	NullableVector() : ValueableVector<T>(), hasValue(false) {};

	inline virtual void Read(INI_EX &parser, const char* pSection, const char* pKey);

	bool HasValue() const {
		return this->hasValue;
	}

	using ValueableVector<T>::GetElements;

	virtual Iterator<T> GetElements(Iterator<T> defElements) const {
		if(!this->hasValue) {
			return defElements;
		}

		return ValueableVector<T>::GetElements();
	}
};

template<typename Lookuper>
class ValueableIdxVector : public ValueableVector<int> {
protected:
	inline virtual void Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue);
};

template<typename Lookuper>
class NullableIdxVector : public NullableVector<int> {
protected:
	inline virtual void Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue);
};

template<typename T>
class ValueableEnum : public Valueable<typename T::Value> {
public:
	typedef typename T::Value ValueType;

	ValueableEnum(ValueType Default = ValueType()) : Valueable<ValueType>(Default) {};

	inline void Read(INI_EX &parser, const char* pSection, const char* pKey);
};

#endif

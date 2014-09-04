#ifndef ARES_TEMPLATEDEF_H
#define ARES_TEMPLATEDEF_H

#include "Template.h"

#include "INIParser.h"
#include "Enums.h"
#include "Constructs.h"

#include <InfantryTypeClass.h>
#include <AircraftTypeClass.h>
#include <UnitTypeClass.h>
#include <BuildingTypeClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <VoxClass.h>


// Valueable

template <typename T>
void Valueable<T>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		const char * val = parser.value();
		if(auto parsed = (Allocate ? base_type::FindOrAllocate : base_type::Find)(val)) {
			this->Set(parsed);
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
};


// ValueableIdx

template <typename Lookuper>
void ValueableIdx<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		const char * val = parser.value();
		int idx = Lookuper::FindIndex(val);
		if(idx != -1 || INIClass::IsBlank(val)) {
			this->Set(idx);
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// NullableIdx

template <typename Lookuper>
void NullableIdx<Lookuper>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		const char * val = parser.value();
		int idx = Lookuper::FindIndex(val);
		if(idx != -1 || INIClass::IsBlank(val)) {
			this->Set(idx);
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
}


// Promotable

template <typename T>
void Promotable<T>::Read(CCINIClass *pINI, const char *Section, const char *BaseFlag) {
	unsigned int buflen = strlen(BaseFlag) + 8;
	char *FlagName = new char[buflen];

	Valueable<T> Placeholder;
	INI_EX exINI(pINI);

	Placeholder.Set(this->Rookie);
	_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Rookie");
	Placeholder.Read(exINI, Section, FlagName);
	this->Rookie = Placeholder;

	Placeholder.Set(this->Veteran);
	_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Veteran");
	Placeholder.Read(exINI, Section, FlagName);
	this->Veteran = Placeholder;

	Placeholder.Set(this->Elite);
	_snprintf_s(FlagName, buflen, buflen - 1, BaseFlag, "Elite");
	Placeholder.Read(exINI, Section, FlagName);
	this->Elite = Placeholder;

	delete[] FlagName;
};


// specializations

template<>
void Valueable<bool>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	bool buffer = this->Get();
	if(parser.ReadBool(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid boolean value [1, true, yes, 0, false, no]");
	}
};

template<>
void Valueable<int>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	int buffer = this->Get();
	if(parser.ReadInteger(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
	}
};

template<>
void Valueable<BYTE>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	int buffer = this->Get();
	if(parser.ReadInteger(pSection, pKey, &buffer)) {
		if(buffer <= 255 && buffer >= 0) {
			const BYTE result(static_cast<byte>(buffer)); // shut up shut up shut up C4244
			this->Set(result);
		} else {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number between 0 and 255 inclusive.");
		}
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid number");
	}
};

template<>
void Valueable<float>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	double buffer = this->Get();
	if(parser.ReadDouble(pSection, pKey, &buffer)) {
		this->Set(static_cast<float>(buffer));
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
	}
};

template<>
void Valueable<double>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	double buffer = this->Get();
	if(parser.ReadDouble(pSection, pKey, &buffer)) {
		this->Set(buffer);
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
	}
};

template<>
void Valueable<ColorStruct>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	ColorStruct buffer = this->Get();
	if(parser.Read3Bytes(pSection, pKey, reinterpret_cast<byte*>(&buffer))) {
		this->Set(buffer);
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid R,G,B color");
	}
};

template<>
void Valueable<CSFText>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		this->Set(parser.value());
	}
};

template<>
void Valueable<SHPStruct *>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		char flag[256];
		const char * val = parser.value();
		_snprintf_s(flag, 255, "%s.shp", val);
		if(SHPStruct *image = FileSystem::LoadSHPFile(flag)) {
			this->Set(image);
		} else {
			Debug::DevLog(Debug::Warning, "Failed to find file %s referenced by [%s]%s=%s", flag, pSection, pKey, val);
		}
	}
};

template<>
void Valueable<MouseCursor>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	Valueable<int> Placeholder;

	MouseCursor *Cursor = this->GetEx();

	// compact way to define the cursor in one go
	if(parser.ReadString(pSection, pKey)) {
		char *buffer = const_cast<char *>(parser.value());
		char *context = nullptr;
		if(char *frame = strtok_s(buffer, Ares::readDelims, &context)) {
			Parser<int>::Parse(frame, &Cursor->Frame);
		}
		if(char *count = strtok_s(nullptr, Ares::readDelims, &context)) {
			Parser<int>::Parse(count, &Cursor->Count);
		}
		if(char *interval = strtok_s(nullptr, Ares::readDelims, &context)) {
			Parser<int>::Parse(interval, &Cursor->Interval);
		}
		if(char *frame = strtok_s(nullptr, Ares::readDelims, &context)) {
			Parser<int>::Parse(frame, &Cursor->MiniFrame);
		}
		if(char *count = strtok_s(nullptr, Ares::readDelims, &context)) {
			Parser<int>::Parse(count, &Cursor->MiniCount);
		}
		if(char *hotx = strtok_s(nullptr, Ares::readDelims, &context)) {
			MouseCursorHotSpotX::Parse(hotx, &Cursor->HotX);
		}
		if(char *hoty = strtok_s(nullptr, Ares::readDelims, &context)) {
			MouseCursorHotSpotY::Parse(hoty, &Cursor->HotY);
		}
	}

	char pFlagName[32];
	_snprintf_s(pFlagName, 31, "%s.Frame", pKey);
	Placeholder.Set(Cursor->Frame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Frame = Placeholder;

	_snprintf_s(pFlagName, 31, "%s.Count", pKey);
	Placeholder.Set(Cursor->Count);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Count = Placeholder;

	_snprintf_s(pFlagName, 31, "%s.Interval", pKey);
	Placeholder.Set(Cursor->Interval);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->Interval = Placeholder;

	_snprintf_s(pFlagName, 31, "%s.MiniFrame", pKey);
	Placeholder.Set(Cursor->MiniFrame);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniFrame = Placeholder;

	_snprintf_s(pFlagName, 31, "%s.MiniCount", pKey);
	Placeholder.Set(Cursor->MiniCount);
	Placeholder.Read(parser, pSection, pFlagName);
	Cursor->MiniCount = Placeholder;

	_snprintf_s(pFlagName, 31, "%s.HotSpot", pKey);
	if(parser.ReadString(pSection, pFlagName)) {
		char *buffer = const_cast<char *>(parser.value());
		char *context = nullptr;
		char *hotx = strtok_s(buffer, ",", &context);
		MouseCursorHotSpotX::Parse(hotx, &Cursor->HotX);

		if(char *hoty = strtok_s(nullptr, ",", &context)) {
			MouseCursorHotSpotY::Parse(hoty, &Cursor->HotY);
		}
	}
};

template<>
void Valueable<RocketStruct>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	Valueable<bool> BoolPlaceholder;
	Valueable<int> IntPlaceholder;
	Valueable<float> FloatPlaceholder;
	Valueable<AircraftTypeClass*> TypePlaceholder;

	RocketStruct* rocket = this->GetEx();

	char pFlagName[0x40];
	_snprintf_s(pFlagName, 0x3F, "%s.PauseFrames", pKey);
	IntPlaceholder.Set(rocket->PauseFrames);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PauseFrames = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.TiltFrames", pKey);
	IntPlaceholder.Set(rocket->TiltFrames);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->TiltFrames = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.PitchInitial", pKey);
	FloatPlaceholder.Set(rocket->PitchInitial);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PitchInitial = FloatPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.PitchFinal", pKey);
	FloatPlaceholder.Set(rocket->PitchFinal);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->PitchFinal = FloatPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.TurnRate", pKey);
	FloatPlaceholder.Set(rocket->TurnRate);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->TurnRate = FloatPlaceholder;

	// sic! integer read like a float.
	_snprintf_s(pFlagName, 0x3F, "%s.RaiseRate", pKey);
	FloatPlaceholder.Set(static_cast<float>(rocket->RaiseRate));
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->RaiseRate = Game::F2I(FloatPlaceholder);

	_snprintf_s(pFlagName, 0x3F, "%s.Acceleration", pKey);
	FloatPlaceholder.Set(rocket->Acceleration);
	FloatPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Acceleration = FloatPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.Altitude", pKey);
	IntPlaceholder.Set(rocket->Altitude);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Altitude = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.Damage", pKey);
	IntPlaceholder.Set(rocket->Damage);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->Damage = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.EliteDamage", pKey);
	IntPlaceholder.Set(rocket->EliteDamage);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->EliteDamage = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.BodyLength", pKey);
	IntPlaceholder.Set(rocket->BodyLength);
	IntPlaceholder.Read(parser, pSection, pFlagName);
	rocket->BodyLength = IntPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.LazyCurve", pKey);
	BoolPlaceholder.Set(rocket->LazyCurve);
	BoolPlaceholder.Read(parser, pSection, pFlagName);
	rocket->LazyCurve = BoolPlaceholder;

	_snprintf_s(pFlagName, 0x3F, "%s.Type", pKey);
	TypePlaceholder.Set(rocket->Type);
	TypePlaceholder.Read(parser, pSection, pFlagName);
	rocket->Type = TypePlaceholder;
};

template <>
void Valueable<Leptons>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool) {
	double buffer = this->Get() / 256.0;
	if(parser.ReadDouble(pSection, pKey, &buffer)) {
		this->Set(Leptons(Game::F2I(buffer * 256.0)));
	} else if(parser.declared()) {
		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a valid floating point number");
	}
}

template<>
void Valueable<OwnerHouseKind>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		auto value = this->Get();

		if(_strcmpi(parser.value(), "default") == 0) {
			value = OwnerHouseKind::Default;
		} else if(_strcmpi(parser.value(), "invoker") == 0) {
			value = OwnerHouseKind::Invoker;
		} else if(_strcmpi(parser.value(), "civilian") == 0) {
			value = OwnerHouseKind::Civilian;
		} else if(_strcmpi(parser.value(), "special") == 0) {
			value = OwnerHouseKind::Special;
		} else if(_strcmpi(parser.value(), "neutral") == 0) {
			value = OwnerHouseKind::Neutral;
		} else {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
			return;
		}

		this->Set(value);
	}
}

// ValueableVector

template <typename T>
void ValueableVector<T>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();
		this->defined = true;
		this->Split(parser, pSection, pKey, Ares::readBuffer);
	}
}

template <typename T>
void ValueableVector<T>::Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue) {
	// if we were able to get the flag in question, take it apart and check the tokens...
	char* context = nullptr;
	for(char *cur = strtok_s(pValue, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
		Parse(parser, pSection, pKey, cur);
	}
}

template <typename T>
void ValueableVector<T>::Parse(INI_EX &parser, const char* pSection, const char* pKey, char* pValue) {
	T buffer = T();
	if(Parser<T>::Parse(pValue, &buffer)) {
		this->push_back(buffer);
	} else if(!INIClass::IsBlank(pValue)) {
		Debug::INIParseFailed(pSection, pKey, pValue);
	}
}


// specializations

template<>
void ValueableVector<TechnoTypeClass *>::Parse(INI_EX &parser, const char* pSection, const char* pKey, char* pValue) {
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


// NullableVector

template <typename T>
void NullableVector<T>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		this->clear();
		this->defined = true;

		// provide a way to reset to default
		if(!_strcmpi(Ares::readBuffer, "<default>")) {
			this->hasValue = false;
		} else {
			this->hasValue = true;
			this->Split(parser, pSection, pKey, Ares::readBuffer);
		}
	}
}


// ValueableIdxVector

template <typename Lookuper>
void ValueableIdxVector<Lookuper>::Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue) {
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


// NullableIdxVector

template <typename Lookuper>
void NullableIdxVector<Lookuper>::Split(INI_EX &parser, const char* pSection, const char* pKey, char* pValue) {
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


// ValueableEnum

template <typename T>
void ValueableEnum<T>::Read(INI_EX &parser, const char* pSection, const char* pKey) {
	if(parser.ReadString(pSection, pKey)) {
		ValueType buffer = this->Get();
		if(T::Parse(Ares::readBuffer, &buffer)) {
			this->Set(buffer);
		} else if(!INIClass::IsBlank(Ares::readBuffer)) {
			Debug::INIParseFailed(pSection, pKey, Ares::readBuffer);
		}
	}
};

#endif

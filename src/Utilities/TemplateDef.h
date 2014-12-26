#ifndef ARES_TEMPLATEDEF_H
#define ARES_TEMPLATEDEF_H

#include "Template.h"

#include "INIParser.h"
#include "Enums.h"
#include "Constructs.h"
#include "../Misc/SavegameDef.h"

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
		auto parsed = (Allocate ? base_type::FindOrAllocate : base_type::Find)(val);
		if(parsed || INIClass::IsBlank(val)) {
			this->Set(parsed);
		} else {
			Debug::INIParseFailed(pSection, pKey, val);
		}
	}
};

template <typename T>
bool Valueable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Savegame::ReadAresStream(Stm, this->Value, RegisterForChange);
}

template <typename T>
bool Valueable<T>::Save(AresStreamWriter &Stm) const {
	return Savegame::WriteAresStream(Stm, this->Value);
}


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


// Nullable

template <typename T>
bool Nullable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	this->Reset();
	auto ret = Savegame::ReadAresStream(Stm, this->HasValue);
	if(ret && this->HasValue) {
		ret = Valueable<T>::Load(Stm, RegisterForChange);
	}
	return ret;
}

template <typename T>
bool Nullable<T>::Save(AresStreamWriter &Stm) const {
	auto ret = Savegame::WriteAresStream(Stm, this->HasValue);
	if(this->HasValue) {
		ret = Valueable<T>::Save(Stm);
	}
	return ret;
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
	char FlagName[0x40];

	Valueable<T> Placeholder;
	INI_EX exINI(pINI);

	Placeholder.Set(this->Rookie);
	_snprintf_s(FlagName, _TRUNCATE, BaseFlag, "Rookie");
	Placeholder.Read(exINI, Section, FlagName);
	this->Rookie = Placeholder;

	Placeholder.Set(this->Veteran);
	_snprintf_s(FlagName, _TRUNCATE, BaseFlag, "Veteran");
	Placeholder.Read(exINI, Section, FlagName);
	this->Veteran = Placeholder;

	Placeholder.Set(this->Elite);
	_snprintf_s(FlagName, _TRUNCATE, BaseFlag, "Elite");
	Placeholder.Read(exINI, Section, FlagName);
	this->Elite = Placeholder;
};

template <typename T>
bool Promotable<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	return Savegame::ReadAresStream(Stm, this->Rookie, RegisterForChange)
		&& Savegame::ReadAresStream(Stm, this->Veteran, RegisterForChange)
		&& Savegame::ReadAresStream(Stm, this->Elite, RegisterForChange);
}

template <typename T>
bool Promotable<T>::Save(AresStreamWriter &Stm) const {
	return Savegame::WriteAresStream(Stm, this->Rookie)
		&& Savegame::WriteAresStream(Stm, this->Veteran)
		&& Savegame::WriteAresStream(Stm, this->Elite);
}


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
		} else if(_strcmpi(parser.value(), "killer") == 0) {
			value = OwnerHouseKind::Killer;
		} else if(_strcmpi(parser.value(), "victim") == 0) {
			value = OwnerHouseKind::Victim;
		} else if(_strcmpi(parser.value(), "civilian") == 0) {
			value = OwnerHouseKind::Civilian;
		} else if(_strcmpi(parser.value(), "special") == 0) {
			value = OwnerHouseKind::Special;
		} else if(_strcmpi(parser.value(), "neutral") == 0) {
			value = OwnerHouseKind::Neutral;
		} else if(_strcmpi(parser.value(), "random") == 0) {
			value = OwnerHouseKind::Random;
		} else {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a owner house kind");
			return;
		}

		this->Set(value);
	}
}

template<>
void Valueable<Mission>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		auto value = MissionControlClass::FindIndex(parser.value());
		if(value != Mission::None) {
			this->Set(value);
		} else if(parser.declared()) {
			Debug::INIParseFailed(pSection, pKey, parser.value(), "Invalid Mission name");
		}
	}
}

template<>
void Valueable<SuperWeaponAITargetingMode>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		static const auto Modes = {
			"none", "nuke", "lightningstorm", "psychicdominator", "paradrop",
			"geneticmutator", "forceshield", "notarget", "offensive", "stealth",
			"self", "base", "multimissile", "hunterseeker", "enemybase"};

		auto it = Modes.begin();
		for(size_t i = 0; i < Modes.size(); ++i) {
			if(_strcmpi(parser.value(), *it++) == 0) {
				this->Set(static_cast<SuperWeaponAITargetingMode>(i));
				return;
			}
		}

		Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a targeting mode");
	}
}

template<>
void Valueable<SuperWeaponTarget>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		auto value = SuperWeaponTarget::None;

		auto str = const_cast<char*>(parser.value());
		char* context = nullptr;
		for(auto cur = strtok_s(str, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			if(!_strcmpi(cur, "land")) {
				value |= SuperWeaponTarget::Land;
			} else if(!_strcmpi(cur, "water")) {
				value |= SuperWeaponTarget::Water;
			} else if(!_strcmpi(cur, "empty")) {
				value |= SuperWeaponTarget::NoContent;
			} else if(!_strcmpi(cur, "infantry")) {
				value |= SuperWeaponTarget::Infantry;
			} else if(!_strcmpi(cur, "units")) {
				value |= SuperWeaponTarget::Unit;
			} else if(!_strcmpi(cur, "buildings")) {
				value |= SuperWeaponTarget::Building;
			} else if(!_strcmpi(cur, "all")) {
				value |= SuperWeaponTarget::All;
			} else if(_strcmpi(cur, "none")) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon target");
				return;
			}
		}

		this->Set(value);
	}
}

template<>
void Valueable<SuperWeaponAffectedHouse>::Read(INI_EX &parser, const char* pSection, const char* pKey, bool Allocate) {
	if(parser.ReadString(pSection, pKey)) {
		auto value = SuperWeaponAffectedHouse::None;

		auto str = const_cast<char*>(parser.value());
		char* context = nullptr;
		for(auto cur = strtok_s(str, Ares::readDelims, &context); cur; cur = strtok_s(nullptr, Ares::readDelims, &context)) {
			if(!_strcmpi(cur, "owner")) {
				value |= SuperWeaponAffectedHouse::Owner;
			} else if(!_strcmpi(cur, "allies")) {
				value |= SuperWeaponAffectedHouse::Allies;
			} else if(!_strcmpi(cur, "enemies")) {
				value |= SuperWeaponAffectedHouse::Enemies;
			} else if(!_strcmpi(cur, "team")) {
				value |= SuperWeaponAffectedHouse::Team;
			} else if(!_strcmpi(cur, "others")) {
				value |= SuperWeaponAffectedHouse::NotOwner;
			} else if(!_strcmpi(cur, "all")) {
				value |= SuperWeaponAffectedHouse::All;
			} else if(_strcmpi(cur, "none")) {
				Debug::INIParseFailed(pSection, pKey, parser.value(), "Expected a super weapon affected house");
				return;
			}
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

template <typename T>
bool ValueableVector<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	size_t size = 0;
	if(Savegame::ReadAresStream(Stm, size, RegisterForChange)) {
		this->clear();
		this->reserve(size);

		for(size_t i = 0; i < size; ++i) {
			value_type buffer = value_type();
			Savegame::ReadAresStream(Stm, buffer, false);
			this->push_back(std::move(buffer));

			if(RegisterForChange) {
				Swizzle swizzle(this->at(i));
			}
		}
		return Savegame::ReadAresStream(Stm, this->defined);
	}
	return false;
}

template <typename T>
bool ValueableVector<T>::Save(AresStreamWriter &Stm) const {
	auto size = this->size();
	if(Savegame::WriteAresStream(Stm, size)) {
		for(size_t i = 0; i < size; ++i) {
			if(!Savegame::WriteAresStream(Stm, this->at(i))) {
				return false;
			}
		}
		return Savegame::WriteAresStream(Stm, this->defined);
	}
	return false;
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

template <typename T>
bool NullableVector<T>::Load(AresStreamReader &Stm, bool RegisterForChange) {
	this->clear();
	this->defined = false;
	if(Savegame::ReadAresStream(Stm, this->hasValue, RegisterForChange)) {
		return !this->hasValue || ValueableVector<T>::Load(Stm, RegisterForChange);
	}
	return false;
}

template <typename T>
bool NullableVector<T>::Save(AresStreamWriter &Stm) const {
	if(Savegame::WriteAresStream(Stm, this->hasValue)) {
		return !this->hasValue || ValueableVector<T>::Save(Stm);
	}
	return false;
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

#endif

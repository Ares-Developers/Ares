#ifndef ARES_MACRO_H
#define ARES_MACRO_H

#define str(x) str_(x)
#define str_(x) #x

#if 0

// DEPRECATED, use Valueable<T> instead

// parse ini faster! harder! stronger!

/*
 * warning: all further funcs depend on
 * const char *section = <section_name_to_read_from>
 * being declared. Since you are not supposed to read from a gajillion sections at once, deal with it.
 */

// WARNING MK II: These funcs are ::Find, not ::FindOrAllocate, make variants if you wish


// find items
#define PARSE_VAR_OBJ(key, var, cls) \
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength) > 0) \
		var = cls::Find(buffer);

#define PARSE_WH(key, var) \
	PARSE_VAR_OBJ(key, var, WarheadTypeClass);

#define PARSE_WEAP(key, var) \
	PARSE_VAR_OBJ(key, var, WeaponTypeClass);

#define PARSE_TECHNO(key, var) \
	PARSE_VAR_OBJ(key, var, TechnoTypeClass);

#define PARSE_AIRCRAFT(key, var) \
	PARSE_VAR_OBJ(key, var, AircraftTypeClass);

#define PARSE_VEHICLE(key, var) \
	PARSE_VAR_OBJ(key, var, UnitTypeClass);

#define PARSE_INFANTRY(key, var) \
	PARSE_VAR_OBJ(key, var, InfantryTypeClass);

#define PARSE_BUILDING(key, var) \
	PARSE_VAR_OBJ(key, var, BuildingTypeClass);

#define PARSE_ANIM(key, var) \
	PARSE_VAR_OBJ(key, var, AnimTypeClass);

#define PARSE_SW(key, var) \
	PARSE_VAR_OBJ(key, var, SuperWeaponTypeClass);

#define PARSE_TASKFORCE(key, var) \
	PARSE_VAR_OBJ(key, var, TaskForceClass);

#define PARSE_SCRIPT(key, var) \
	PARSE_VAR_OBJ(key, var, ScriptTypeClass);

#define PARSE_TEAM(key, var) \
	PARSE_VAR_OBJ(key, var, TeamTypeClass);


// find indices
#define PARSE_VAR_IDX(key, var, cls) \
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength) > 0) \
		var = cls::FindIndex(buffer);

#define PARSE_SND(key, var) \
	PARSE_VAR_IDX(key, var, VocClass);

#define PARSE_EVA(key, var) \
	PARSE_VAR_IDX(key, var, VoxClass);

#define PARSE_WH_IDX(key, var) \
	PARSE_VAR_IDX(key, var, WarheadTypeClass);

#define PARSE_WEAP_IDX(key, var) \
	PARSE_VAR_IDX(key, var, WeaponTypeClass);

#define PARSE_TECHNO_IDX(key, var) \
	PARSE_VAR_IDX(key, var, TechnoTypeClass);

#define PARSE_AIRCRAFT_IDX(key, var) \
	PARSE_VAR_IDX(key, var, AircraftTypeClass);

#define PARSE_VEHICLE_IDX(key, var) \
	PARSE_VAR_IDX(key, var, UnitTypeClass);

#define PARSE_INFANTRY_IDX(key, var) \
	PARSE_VAR_IDX(key, var, InfantryTypeClass);

#define PARSE_BUILDING_IDX(key, var) \
	PARSE_VAR_IDX(key, var, BuildingTypeClass);

#define PARSE_ANIM_IDX(key, var) \
	PARSE_VAR_IDX(key, var, AnimTypeClass);

#define PARSE_SW_IDX(key, var) \
	PARSE_VAR_IDX(key, var, SuperWeaponTypeClass);

#define PARSE_TASKFORCE_IDX(key, var) \
	PARSE_VAR_IDX(key, var, TaskForceClass);

#define PARSE_SCRIPT_IDX(key, var) \
	PARSE_VAR_IDX(key, var, ScriptTypeClass);

#define PARSE_TEAM_IDX(key, var) \
	PARSE_VAR_IDX(key, var, TeamTypeClass);


// find Customizable<>s
// by value
#define PARSE_VAR_EX(key, var, cls) \
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength) > 0) \
		var.Set(cls::Find(buffer));

#define PARSE_WH_EX(key, var) \
	PARSE_VAR_EX(key, var, WarheadTypeClass);

#define PARSE_WEAP_EX(key, var) \
	PARSE_VAR_EX(key, var, WeaponTypeClass);

#define PARSE_TECHNO_EX(key, var) \
	PARSE_VAR_EX(key, var, TechnoTypeClass);

#define PARSE_AIRCRAFT_EX(key, var) \
	PARSE_VAR_EX(key, var, AircraftTypeClass);

#define PARSE_VEHICLE_EX(key, var) \
	PARSE_VAR_EX(key, var, UnitTypeClass);

#define PARSE_INFANTRY_EX(key, var) \
	PARSE_VAR_EX(key, var, InfantryTypeClass);

#define PARSE_BUILDING_EX(key, var) \
	PARSE_VAR_EX(key, var, BuildingTypeClass);

#define PARSE_ANIM_EX(key, var) \
	PARSE_VAR_EX(key, var, AnimTypeClass);

#define PARSE_SW_EX(key, var) \
	PARSE_VAR_EX(key, var, SuperWeaponTypeClass);

#define PARSE_TASKFORCE_EX(key, var) \
	PARSE_VAR_EX(key, var, TaskForceClass);

#define PARSE_SCRIPT_EX(key, var) \
	PARSE_VAR_EX(key, var, ScriptTypeClass);

#define PARSE_TEAM_EX(key, var) \
	PARSE_VAR_EX(key, var, TeamTypeClass);


// by index
#define PARSE_VAR_IDX_EX(key, var, cls) \
	if(pINI->ReadString(section, key, "", Ares::readBuffer, Ares::readLength) > 0) \
		var.Set(cls::FindIndex(buffer));

#define PARSE_SND_EX(key, var) \
	PARSE_VAR_IDX_EX(key, var, VocClass);


// read colors
#define PARSE_COLOR(key, var, buf) \
		buf = var; \
		pINI->ReadColor(&buf, section, key, &var); \
		var = buf;

#endif

//
#define IF_STR(section, key) \
	if(INI->ReadString(section, key, Ares::readDefval, Ares::readBuffer, Ares::readLength))
#define FOR_STRTOK \
	for(char *cur = strtok(Ares::readBuffer, Ares::readDelims); \
		cur; cur = strtok(NULL, Ares::readDelims))

#define PARSE_VECTOR(ini_section, ini_key, var, objtype) \
IF_STR(ini_section, #ini_key) { \
	DynamicVectorClass<objtype *>* vec = var; vec->Clear(); \
	FOR_STRTOK{ \
		objtype *idx = objtype::Find(cur); if(idx) { vec->AddItem(idx); } \
	} \
}

#define PARSE_VECTOR_N(ini_section, obj, ini_key, objtype) \
IF_STR(ini_section, #ini_key) { \
	DynamicVectorClass<objtype *>* vec = obj->get_ ## ini_key(); vec->Clear(); \
	FOR_STRTOK{ \
		objtype *idx = objtype::Find(cur); \
		if(idx) { vec->AddItem(idx); } \
	} \
}

#define PARSE_VECTOR_BIT(ini_section, obj, ini_key, objtype, obj_key) \
IF_STR(ini_section, #ini_key) { \
	DWORD buf = 0; \
	FOR_STRTOK{ \
		int idx = objtype::FindIndex(cur); if(idx > -1) { buf |= (1 << idx); } \
	} \
	obj-> ## obj_key  = buf; \
}

#define PARSE_VECTOR_INT(ini_section, ini_key, obj) \
IF_STR(ini_section, #ini_key) { \
	DynamicVectorClass<int>* vec = obj->get_ ## ini_key(); vec->Clear(); \
	FOR_STRTOK{ \
		int idx = atoi(cur); vec->AddItem(idx); \
	} \
}

#endif

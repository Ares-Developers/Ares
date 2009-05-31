#ifndef ARES_MACRO_H
#define ARES_MACRO_H

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
	obj->set_ ## obj_key (buf); \
}

#define PARSE_VECTOR_INT(ini_section, ini_key, obj) \
IF_STR(ini_section, #ini_key) { \
	DynamicVectorClass<int>* vec = obj->get_ ## ini_key(); vec->Clear(); \
	FOR_STRTOK{ \
		int idx = atoi(cur); vec->AddItem(idx); \
	} \
}

#endif

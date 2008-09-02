#include "Exceptions.h"
#include "Ares.h"

// 74FDC0, 5
EXPORT_FUNC(Exception_Log)
{
	R->set_EAX((DWORD)Ares::Version);
	return 0x74FEEF;
}

// 4C8BC7, 6
EXPORT_FUNC(Exception_Log_RegContent)
{
	GET(_CONTEXT *, ctxt, EBP);
	char buffer [256];  // IDs can't exceed some 24 chars, this should be enough
	const char format[] = "%s: %08X - [%s]\n";

#define LOG_REG(REG, reg) \
	sprintf(buffer, format, # REG, ctxt-> reg, Exceptions::PointerToID(ctxt->reg)); \
	lstrcatA(Unsorted::except_txt_content, buffer);

	LOG_REG(EAX, Eax);
	LOG_REG(EBX, Ebx);
	LOG_REG(ECX, Ecx);
	LOG_REG(EDX, Edx);
	LOG_REG(ESI, Esi);
	LOG_REG(EDI, Edi);
	return 0x4C8C63;
}

// 4C8E54, 9
EXPORT_FUNC(Exception_Log_StackContent)
{
	GET(DWORD *, ptr, EDI);
	char buffer [256];
	const char format[] = "%08X: %08X [%s]\n";

	sprintf(buffer, format, ptr, *ptr, Exceptions::PointerToID(*ptr));
	if(lstrlenA(Unsorted::except_txt_content) + lstrlenA(buffer) < Unsorted::except_txt_length)
	{
		lstrcatA(Unsorted::except_txt_content, buffer);
		return 0x4C8F4C;
	}
	return 0x4C8F46;
}

// types
#define FIND_PTR_IN_T(arr, ptr) \
	for(int i = 0; i < arr ## TypeClass::Array->get_Count(); ++i) \
		if((DWORD)arr ## TypeClass::Array->GetItem(i) == ptr) \
			return arr ## TypeClass::Array->GetItem(i)->get_ID();

// instances
#define FIND_PTR_IN_I(arr, ptr) \
	for(int i = 0; i < arr ## Class::Array->get_Count(); ++i) \
		if((DWORD)arr ## Class::Array->GetItem(i) == ptr) \
			return arr ## Class::Array->GetItem(i)->GetType()->get_ID();

// both
#define FIND_PTR(arr, ptr) \
	FIND_PTR_IN_T(arr, ptr); \
	FIND_PTR_IN_I(arr, ptr);

const char *Exceptions::PointerToID(DWORD ptr)
{
	FIND_PTR(Aircraft, ptr);
	FIND_PTR(Building, ptr);
	FIND_PTR(Infantry, ptr);
	FIND_PTR(Unit, ptr);
	FIND_PTR(Bullet, ptr);
	FIND_PTR(Anim, ptr);
	FIND_PTR(Overlay, ptr);
	FIND_PTR(Particle, ptr);
	FIND_PTR(ParticleSystem, ptr);
	FIND_PTR(Smudge, ptr);

	FIND_PTR_IN_T(SuperWeapon, ptr);
	FIND_PTR_IN_T(Weapon, ptr);
	FIND_PTR_IN_T(Warhead, ptr);
	FIND_PTR_IN_T(House, ptr);
	return "not an object";
}
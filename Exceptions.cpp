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
	char text_buffer[256] = "\0";
	const char format[] = "%s: %08X - [%s]\n";

#define LOG_REG(REG, reg) \
	sprintf(buffer, format, # REG, ctxt-> reg, Exceptions::PointerToText(ctxt->reg, text_buffer)); \
	lstrcatA(Unsorted::except_txt_content, buffer);

	LOG_REG(EAX, Eax);
	LOG_REG(ECX, Ecx);
	LOG_REG(EDX, Edx);
	LOG_REG(EBX, Ebx);
	LOG_REG(ESP, Esp);
	LOG_REG(EBP, Ebp);
	LOG_REG(ESI, Esi);
	LOG_REG(EDI, Edi);
	return 0x4C8C63;
}

// 4C8E54, 9
EXPORT_FUNC(Exception_Log_StackContent)
{
	GET(DWORD *, ptr, EDI);
	char buffer [256];
	char text_buffer[256] = "\0";
	const char format[] = "%08X: %08X [%s]\n";

	sprintf(buffer, format, ptr, *ptr, Exceptions::PointerToText(*ptr, text_buffer));
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

char *Exceptions::PointerToText(DWORD ptr, char* out)
{
	//we assume now that ptr is a pointer to an object that has a VTable pointer at offset 0
	//if anything goes wrong, continue
	DWORD* p = (DWORD*)ptr;
	if(!IsBadReadPtr(p, 0x4))
	{
		DWORD* VTable = (DWORD*)*p;
		if(!IsBadReadPtr(--VTable, 0x4))
		{
			DWORD* RTTI = (DWORD*)*VTable;
			if(!IsBadReadPtr(RTTI, 0x10))
			{
				DWORD* TypeDesc = (DWORD*)RTTI[3];
				if(!IsBadReadPtr(TypeDesc, 0x8))
				{
					if(*TypeDesc == 0x7F9594)
					{
						//it is! return mangled class name
						char* MangledName = (char*)TypeDesc + 8;

						//browse base classes and retrieve more info if possible
						DWORD* Bases = (DWORD*)RTTI[4];
						
						int NumBases = (int)Bases[2];
						DWORD** BasesList = (DWORD**)Bases[3];
						for(int i = 0; i < NumBases; i++)
						{
							DWORD* currentBase = BasesList[i];
							if(*currentBase == 0x817808)
							{
								//p is an AbstractTypeClass subclass!
								AbstractTypeClass* AT = (AbstractTypeClass*)p;
								sprintf(out, "%s - %s", MangledName, AT->get_ID());
								return out;
							}
							else if(*currentBase == 0x817AF8)
							{
								//p is an ObjectClass subclass!
								ObjectClass* O = (ObjectClass*)p;
								ObjectTypeClass* OT = O->GetType();

								if(OT)
									sprintf(out, "%s - Type: %s", MangledName, OT->get_ID());
								else
									sprintf(out, "%s - Type: NULL", MangledName);

								return out;
							}
							else if(*currentBase == 0x8177C8)
							{
								//p is an AbstractClass subclass!
								AbstractClass* A = (AbstractClass*)p;
								break;
							}
						}
					}
				}
			}
		}
	}

	if(!IsBadReadPtr(p, 1))
		strcpy(out, "Pointer");
	else
		strcpy(out, "Data");

	return out;
}
#include "Exceptions.h"

// 74FDC0, 5
EXPORT_FUNC(Exception_Log)
{
	R->set_EAX((DWORD)VERSION_STRING);
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

AbstractClass* Exceptions::PointerToAbstract(DWORD p)
{
	if(p == NULL)
		return NULL;

	int n = -1;

	//search most Abstracts
	AbstractClass* q = (AbstractClass*)p;
	Ares::Log("\tSearching AbstractClass array...\n");

	DynamicVectorClass<AbstractClass*>* pArray = 
		(DynamicVectorClass<AbstractClass*>*)0xB0F670;

	for(int i = 0; i < pArray->get_Count(); i++)
	{
		AbstractClass* A = pArray->GetItem(i);
		if((DWORD)A == p)
			return A;
	}
	/* this causes errors, I have no idea why...
	n = pArray->FindItemIndex((AbstractClass*)p);
	if(n >= 0)
		return pArray->GetItem(n);
	*/

	//search AbstractTypes
	Ares::Log("\tSearching AbstractTypeClass array...\n");

	for(int i = 0; i < AbstractTypeClass::Array->get_Count(); i++)
	{
		AbstractTypeClass* AT = AbstractTypeClass::Array->GetItem(i);
		if((DWORD)AT == p)
			return AT;
	}
	/*
	n = AbstractTypeClass::Array->FindItemIndex((AbstractTypeClass*)p);
	if(n >= 0)
		return AbstractTypeClass::Array->GetItem(n);
	*/

	//nothing found
	return NULL;
}

char *Exceptions::PointerToText(DWORD ptr, char* out)
{
	Ares::Log("\tTrying to resolve 0x%08X...\n", ptr);

	if(ptr)
	{
		AbstractClass* A = PointerToAbstract(ptr);

		Ares::Log("\tA = 0x%08X\n", A);

		if(A)
		{
			AbstractTypeClass* AT = dynamic_cast<AbstractTypeClass*>(A);
			Ares::Log("\tAT = 0x%08X\n", AT);
			if(AT)
				sprintf(out, "%s: %s", AT->GetClassName(), AT->get_ID());
			else
			{
				ObjectClass* O = dynamic_cast<ObjectClass*>(A);
				Ares::Log("\tO = 0x%08X\n", O);
				if(O)
				{
					ObjectTypeClass* OT = O->GetType();
					Ares::Log("\tOT = 0x%08X\n", O);
					if(OT)
						sprintf(out, "%s of type %s", O->GetClassName(), OT->get_ID());
					else
						strcpy(out, O->GetClassName());
				}
				else
					strcpy(out, A->GetClassName());
			}
		}
		else
			strcpy(out, "Unknown");
	}
	else
		strcpy(out, "NULL");

	Ares::Log("\tout -> %s\n\n", out);

	return out;
}
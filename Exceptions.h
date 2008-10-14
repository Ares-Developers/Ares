#ifndef EXCEPT_H
#define EXCEPT_H

#include "Ares.Version.h"

#include <MacroHelpers.h> //basically indicates that this is DCoder country
#include <ObjectClass.h>
#include <ObjectTypeClass.h>
#include <Unsorted.h>

class Exceptions
{
private:
	static AbstractClass* PointerToAbstract(DWORD p);

public:
	static char* PointerToText(DWORD ptr, char* out);
};

#endif

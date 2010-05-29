#ifndef INISECTIONINCLUDES_H
#define INISECTIONINCLUDES_H

#include <CCINIClass.h>

//temporary information holder
class IniSectionIncludes
{
public:
	static INIClass::INISection* includedSection;

	static CCINIClass::INISection* PreProcess(CCINIClass* ini, char* str);
	static void CopySection(CCINIClass* ini, INIClass::INISection* source, const char* dest);
};

#endif

#ifndef EVAVOICES_H
#define EVAVOICES_H

#include <VoxClass.h>

struct VoxFile
{
	char Name[9];

	//need to define a == operator so it can be used in array classes
	bool operator == (VoxFile &other)
	{
		return !strcmp(this->Name, other.Name);
	}
};

class VoxClass2 : public VoxClass
{
public:
	VoxClass2(char* pName) : VoxClass(pName) {}

	~VoxClass2()
	{
		Voices.clear();
		this->VoxClass::~VoxClass();
	}

	std::vector<VoxFile> Voices;
};

class EVAVoices
{
public:
	static int FindIndex(const char* type)
	{
		// the default values
		if(!_strcmpi(type, "Allied")) {
			return 0;
		} else if(!_strcmpi(type, "Russian")) {
			return 1;
		} else if(!_strcmpi(type, "Yuri")) {
			return 2;
		}

		// find all others
		for(unsigned int i=0; i<Types.size(); ++i) {
			if(!_strcmpi(type, Types[i])) {
				return i + 3;
			}
		}

		// not found
		return -1;
	}

	// adds the EVA type only if it doesn't exist
	static void RegisterType(const char* type)
	{
		int index = FindIndex(type);

		if(index < 0) {
			const char* copy = _strdup(type);
			Types.push_back(copy);
		}
	}

	static std::vector<const char*> Types;
};

#endif

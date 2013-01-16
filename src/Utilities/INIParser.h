#ifndef ARES_INI_PARSER_H
#define ARES_INI_PARSER_H

#include "..\Ares.h"
#include <CCINIClass.h>

class INI_EX {
	CCINIClass* pINI;

	char * buffer() {
		return Ares::readBuffer;
	}

	int buflen() {
		return Ares::readLength;
	}

public:
	INI_EX(CCINIClass *iniFile)
	 : pINI(iniFile)
	{};

	bool ReadString(const char* pSection, const char* pKey) {
		return pINI->ReadString(pSection, pKey, "", this->buffer(), this->buflen()) >= 1;
	}

	const char * value() {
		return Ares::readBuffer;
	}

	bool declared() const {
		return !!Ares::readBuffer[0];
	}

	static bool IsBlank(const char *pValue = Ares::readBuffer) {
		return !_strcmpi(pValue, "<none>") || !_strcmpi(pValue, "none");
	}

	// bool
	bool ReadBool(const char* pSection, const char* pKey, bool* bBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		*bBuffer = this->ParseBool(*bBuffer);
		return true;
	}

	bool ParseBool(const bool bDefault) {
		switch(toupper(*this->buffer())) {
			case '1':
			case 'T':
			case 'Y':
				return true;
			case '0':
			case 'F':
			case 'N':
				return false;
			default:
				return bDefault;
		}
	}


	bool ReadInteger(const char* pSection, const char* pKey, int* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		*nBuffer = this->ParseInteger(*nBuffer);
		return true;
	}

	int ParseInteger(const int nDefault) {
		int buffer = nDefault;

		const char *pValue = this->buffer();

		char *pFmt = NULL;

		if(*pValue == '$') {
			pFmt = "$%d";
		} else if(tolower(pValue[strlen(pValue) - 2]) == 'h') {
			pFmt = "%xh";
		} else {
			pFmt = "%d";
		}

		if(sscanf(pValue, pFmt, &buffer) != 1) {
			buffer = nDefault;
		}
		return buffer;
	}


	bool Read2Integers(const char* pSection, const char* pKey, int* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		this->Parse2Integers(nBuffer);
		return true;
	}

	int* Parse2Integers(int *buffer) {
		int nDefault[2] = { buffer[0], buffer[1] };
		switch(sscanf(this->buffer(), "%d,%d", &buffer[0], &buffer[1])) {
			case 0:
				buffer[0] = nDefault[0];
				// fallthrough
			case 1:
				buffer[1] = nDefault[1];
		}
		return buffer;
	}


	bool Read3Integers(const char* pSection, const char* pKey, int* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		this->Parse3Integers(nBuffer);
		return true;
	}

	int* Parse3Integers(int *buffer) {
		int nDefault[3] = { buffer[0], buffer[1], buffer[2] };
		switch(sscanf(this->buffer(), "%d,%d,%d", &buffer[0], &buffer[1], &buffer[2])) {
		case 0:
			buffer[0] = nDefault[0];
			// fallthrough
		case 1:
			buffer[1] = nDefault[1];
			// fallthrough
		case 2:
			buffer[2] = nDefault[2];
		}
		return buffer;
	}


	bool Read4Integers(const char* pSection, const char* pKey, int* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		this->Parse4Integers(nBuffer);
		return true;
	}

	int* Parse4Integers(int *buffer) {
		int nDefault[4] = { buffer[0], buffer[1], buffer[2], buffer[3] };
		switch(sscanf(this->buffer(), "%d,%d,%d,%d", &buffer[0], &buffer[1], &buffer[2], &buffer[3])) {
		case 0:
			buffer[0] = nDefault[0];
			// fallthrough
		case 1:
			buffer[1] = nDefault[1];
			// fallthrough
		case 2:
			buffer[2] = nDefault[2];
			// fallthrough
		case 3:
			buffer[3] = nDefault[3];
		}
		return buffer;
	}


	bool Read3Bytes(const char* pSection, const char* pKey, byte* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		this->Parse3Bytes(nBuffer);
		return true;
	}

	byte* Parse3Bytes(byte *buffer) {
		byte nDefault[3] = { buffer[0], buffer[1], buffer[2] };
		int iBuffer[3] = { buffer[0], buffer[1], buffer[2] };

		switch(sscanf(this->buffer(), "%d,%d,%d", &iBuffer[0], &iBuffer[1], &iBuffer[2])) {
			case 0:
				buffer[0] = nDefault[0];
				// fallthrough
			case 1:
				buffer[1] = nDefault[1];
				// fallthrough
			case 2:
				buffer[2] = nDefault[2];
		}

		buffer[0] = (byte)iBuffer[0];
		buffer[1] = (byte)iBuffer[1];
		buffer[2] = (byte)iBuffer[2];
		return buffer;
	}


	bool ReadDouble(const char* pSection, const char* pKey, double* nBuffer) {
		if(!this->ReadString(pSection, pKey)) {
			return false;
		}
		*nBuffer = this->ParseDouble(*nBuffer);
		return true;
	}

	double ParseDouble(double nDefault) {
		double buffer = nDefault;
		if(sscanf(this->buffer(), "%lf", &buffer) == 1) {
			if(strchr(this->buffer(), '%')) {
				buffer *= 0.01;
			}
		} else {
			buffer = nDefault;
		}
		return buffer;
	}

};
#endif

#include <YRPP.h>

//595BD8, 0
EXPORT RMG_ReplaceDialog(REGISTERS* R)
{
	PUSH_IMM(0); //no idea
	SET_REG32(edx, 0x596300); //dialog function
	SET_REG32(ecx, 5000); //our new dialog
	CALL(0x622650); //show dialog

	return 0x595BE8;
}
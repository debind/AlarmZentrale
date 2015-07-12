#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dispsw.h>

#include "types.h"
#include "main.h"


void funcMenu15(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)
{
	switch(u8Cmd)
	{
		case DISP:
		case FLASH:
			dispsw_Set(u8MenuNumber/10, u8MenuNumber%10, u8MenuValue/10, u8MenuValue%10);
			break;
		case SET:
			break;
	}
}

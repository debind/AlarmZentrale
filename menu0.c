#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dispsw.h>

#include "main.h"


//******************************************************************************
// Menu0: Clock
//******************************************************************************
void funcMenu0(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)
{
	UINT8 u8Z3, u8Z2, u8Z1, u8Z0;	
	struct tm *tmp;
	time_t s;

	s = time(NULL);
	tmp = localtime(&s);

	tAlarmCenter.iHour = tmp->tm_hour;
	tAlarmCenter.iMin  = tmp->tm_min;

	u8Z3 = tAlarmCenter.iHour / 10;
	u8Z2 = tAlarmCenter.iHour % 10;
	u8Z1 = tAlarmCenter.iMin  / 10;
	u8Z0 = tAlarmCenter.iMin  % 10;

	dispsw_Set(u8Z3, u8Z2, u8Z1, u8Z0);	

}

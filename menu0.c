#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dispsw.h>



//******************************************************************************
// Menu0: Clock
//******************************************************************************
void funcMenu0(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)
{
	UINT8 u8Z3, u8Z2, u8Z1, u8Z0;
	int ihour, imin;
	struct tm *tmp;
	time_t s;

	s = time(NULL);
	tmp = localtime(&s);

	ihour = tmp->tm_hour;
	imin  = tmp->tm_min;

	u8Z3 = ihour / 10;
	u8Z2 = ihour % 10;
	u8Z1 = imin  / 10;
	u8Z0 = imin  % 10;

	dispsw_Set(u8Z3, u8Z2, u8Z1, u8Z0);	

}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>



//--------------------------------------------------
// local function prototypes
//--------------------------------------------------

//--------------------------------------------------
// global vars
//--------------------------------------------------

//--------------------------------------------------
// main entry
//--------------------------------------------------
int main(int argc, char * argv[])
{
	int i;
	char c;

	c = getc(stdin);

	printf("0x%02x\n", c);

	return 0;
}



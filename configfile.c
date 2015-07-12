#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <iniparser.h>

#include "types.h"
#include "main.h"


//--------------------------------------------------
// create a default conf file
//--------------------------------------------------
void configfile_CreateDefault(void)
{
    FILE    *   ini ;
    ini = fopen(CONF_FILE, "w");
    fprintf(ini,
    "#\n"
    "# configuration file alarmcenter.conf\n"
    "#\n"
    "\n"
    "[Main]\n"
    "OnOff     = 0 ;\n"
    "AlarmMode = 0 ;\n"
    "WarningMode = 0 ;\n"
    "WarningTime = 0 ;\n"
    "AlarmTime = 0 ;\n"
    "AlarmSchwelle1 = 0 ;\n"
    "AlarmSchwelle2 = 0 ;\n"
    "\n"
    "[Tcp]\n"
    "Port = 12345 ;\n"
    "\n");
    fclose(ini);
}

//--------------------------------------------------
// read the conf file
//--------------------------------------------------
int configfile_Read(char * ini_name, ALARMCENTER* ptAlarmCenter)
{
    dictionary  *   ini ;

    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        fprintf(stderr, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }
 
    ptAlarmCenter->iOnOff            = iniparser_getint(ini, "Main:OnOff", 0);
	ptAlarmCenter->iAlarmMode        = iniparser_getint(ini, "Main:AlarmMode", 1);
	ptAlarmCenter->iAlarmTime        = iniparser_getint(ini, "Main:AlarmTime", 10);
	ptAlarmCenter->iWarningMode      = iniparser_getint(ini, "Main:WarningMode", 1);
	ptAlarmCenter->iWarningTime      = iniparser_getint(ini, "Main:WarningTime", 5);
	ptAlarmCenter->iAlarmSchwelle1   = iniparser_getint(ini, "Main:AlarmSchwelle1", 1);
	ptAlarmCenter->iAlarmSchwelle2   = iniparser_getint(ini, "Main:AlarmSchwelle2", 50);

	ptAlarmCenter->iTcpPort          = iniparser_getint(ini, "Tcp:Port", 1234);
    
    iniparser_freedict(ini);
    return 0 ;
}

//--------------------------------------------------
// Config File Safe
//--------------------------------------------------
int configfile_Safe(char * ini_name, ALARMCENTER* ptAlarmCenter)
{
	char txt[500];
	FILE* fTest;
    dictionary  *   ini ;

    ini = iniparser_load(ini_name);
    if (ini==NULL) {
        fprintf(stderr, "cannot parse file: %s\n", ini_name);
        return -1 ;
    }
 
	sprintf(txt, "%d", ptAlarmCenter->iOnOff)         ; iniparser_set(ini, "Main:OnOff", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmMode)     ; iniparser_set(ini, "Main:AlarmMode", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmTime)     ; iniparser_set(ini, "Main:AlarmTime", txt);
	sprintf(txt, "%d", ptAlarmCenter->iWarningMode)   ; iniparser_set(ini, "Main:WarningMode", txt);
	sprintf(txt, "%d", ptAlarmCenter->iWarningTime)   ; iniparser_set(ini, "Main:WarningTime", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmSchwelle1); iniparser_set(ini, "Main:AlarmSchwelle1", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmSchwelle2); iniparser_set(ini, "Main:AlarmSchwelle2", txt);
	sprintf(txt, "%d", ptAlarmCenter->iTcpPort)       ; iniparser_set(ini, "Tcp:Port", txt);
    
	fTest = fopen(ini_name,"w");
	if (fTest != NULL)	iniparser_dump_ini (ini, fTest);

    iniparser_freedict(ini);
    return 0 ;
}


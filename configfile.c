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
    "[main]\n"
    "onoff     = 0 ;\n"
    "alarmmode = 0 ;\n"
    "warningmode = 0 ;\n"
    "warningtime = 0 ;\n"
    "alarmtime = 0 ;\n"
    "alarmschwelle1 = 0 ;\n"
    "alarmschwelle2 = 0 ;\n"
	"ondelay = 10; \n"
	"alarmdelay = 5; \n"
	"warningdelay = 5; \n"
    "\n"
    "[tcp]\n"
    "port = 12345 ;\n"
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
 
    ptAlarmCenter->iOnOff            = iniparser_getint(ini, "main:onoff", 0);
	ptAlarmCenter->iAlarmMode        = iniparser_getint(ini, "main:alarmmode", 1);
	ptAlarmCenter->iAlarmTime        = iniparser_getint(ini, "main:alarmtime", 10);
	ptAlarmCenter->iWarningMode      = iniparser_getint(ini, "main:warningmode", 1);
	ptAlarmCenter->iWarningTime      = iniparser_getint(ini, "main:warningtime", 5);
	ptAlarmCenter->iAlarmSchwelle1   = iniparser_getint(ini, "main:alarmschwelle1", 1);
	ptAlarmCenter->iAlarmSchwelle2   = iniparser_getint(ini, "main:alarmschwelle2", 50);
	ptAlarmCenter->iOnDelay          = iniparser_getint(ini, "main:ondelay", 50);
	ptAlarmCenter->iAlarmDelay       = iniparser_getint(ini, "main:alarmdelay", 50);
	ptAlarmCenter->iWarningDelay     = iniparser_getint(ini, "main:warningdelay", 50);

	ptAlarmCenter->iTcpPort          = iniparser_getint(ini, "tcp:port", 1234);
    
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
 
	sprintf(txt, "%d", ptAlarmCenter->iOnOff)         ; iniparser_set(ini, "main:onoff", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmMode)     ; iniparser_set(ini, "main:alarmmode", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmTime)     ; iniparser_set(ini, "main:alarmtime", txt);
	sprintf(txt, "%d", ptAlarmCenter->iWarningMode)   ; iniparser_set(ini, "main:warningmode", txt);
	sprintf(txt, "%d", ptAlarmCenter->iWarningTime)   ; iniparser_set(ini, "main:warningtime", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmSchwelle1); iniparser_set(ini, "main:alarmschwelle1", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmSchwelle2); iniparser_set(ini, "main:alarmschwelle2", txt);
	sprintf(txt, "%d", ptAlarmCenter->iOnDelay)       ; iniparser_set(ini, "main:ondelay", txt);
	sprintf(txt, "%d", ptAlarmCenter->iAlarmDelay)    ; iniparser_set(ini, "main:alarmdelay", txt);
	sprintf(txt, "%d", ptAlarmCenter->iWarningDelay)  ; iniparser_set(ini, "main:warningdelay", txt);
	sprintf(txt, "%d", ptAlarmCenter->iTcpPort)       ; iniparser_set(ini, "Tcp:Port", txt);
    
	fTest = fopen(ini_name,"w");
	if (fTest != NULL)	iniparser_dump_ini (ini, fTest);

    iniparser_freedict(ini);
    return 0 ;
}


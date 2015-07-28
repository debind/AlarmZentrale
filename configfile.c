#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <iniparser.h>

#include "types.h"
#include "main.h"
#include "configfile.h"


//--------------------------------------------------
// create a default conf file
//--------------------------------------------------
void configfile_CreateDefault(ALARMCENTER* ptAlarmCenter)
{
    ptAlarmCenter->iOnOff            = 1;
	ptAlarmCenter->iAlarmMode        = 5;
	ptAlarmCenter->iAlarmTime        = 10;
	ptAlarmCenter->iAlarmSchwelle2   = 50;
	ptAlarmCenter->iWarningMode      = 1;
	ptAlarmCenter->iWarningTime      = 10;
	ptAlarmCenter->iAlarmSchwelle1   = 15;
	ptAlarmCenter->iOnDelay          = 10;
	ptAlarmCenter->iAlarmDelay       = 5;
	ptAlarmCenter->iWarningDelay     = 5;
	ptAlarmCenter->iTcpPort          = 2345;
    
	configfile_Safe(CONF_FILE, ptAlarmCenter);
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
	FILE* fIni;

	fIni = fopen(ini_name,"w+");
	if (fIni == NULL)	return 1;
	
	fprintf(fIni, "[main]\n");
	fprintf(fIni, "onoff = %d\n", ptAlarmCenter->iOnOff);
	fprintf(fIni, "alarmmode = %d\n", ptAlarmCenter->iAlarmMode);
	fprintf(fIni, "alarmtime = %d\n", ptAlarmCenter->iAlarmTime);
	fprintf(fIni, "alarmschwelle2 = %d\n", ptAlarmCenter->iAlarmSchwelle2);
	fprintf(fIni, "warningmode = %d\n", ptAlarmCenter->iWarningMode);
	fprintf(fIni, "warningtime = %d\n", ptAlarmCenter->iWarningTime);
	fprintf(fIni, "alarmschwelle1 = %d\n", ptAlarmCenter->iAlarmSchwelle1);
	fprintf(fIni, "ondelay = %d\n", ptAlarmCenter->iOnDelay);
	fprintf(fIni, "alarmdelay = %d\n", ptAlarmCenter->iAlarmDelay);
	fprintf(fIni, "warningdelay = %d\n", ptAlarmCenter->iWarningDelay);
	fprintf(fIni, "\n");
	fprintf(fIni, "[tcp]\n");
	fprintf(fIni, "port = %d\n", ptAlarmCenter->iTcpPort);
 
    fclose(fIni);

    return 0 ;
}


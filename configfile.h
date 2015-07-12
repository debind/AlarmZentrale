#ifndef CONFIGFILE_H_
#define CONFIGFILE_H_


extern void configfile_CreateDefault(void);
extern int  configfile_Read         (char * ini_name, ALARMCENTER* ptAlarmCenter);
extern int  configfile_Safe         (char * ini_name, ALARMCENTER* ptAlarmCenter);







#endif

#ifndef MAIN_H_
#define MAIN_H_

//--------------------------------------------------
// defines
#define CONF_FILE          "/home/pi/alarmcenter.conf"
#define LOG_FILE           "/home/pi/alarmcenter.log"

#define CMD_THREADCYCLE    (50)  // [ms]
#define MAINCYCLE          (50)  // [ms]

#define CMD_ITEM_SIZE      (100)
//--------------------------------------------------

//--------------------------------------------------
// typedefs / structs
typedef struct tagAlarmCenter
{
	int iOnOff;           // Alarmzentrale on/off
	int iAlarmTime;       // länge des alarm wenn aktiv [sec]
	int iAlarmMode;       // mode des alarm
	int iWarningTime;     // [sec]
	int iWarningMode;
	int iAlarmSchwelle1;
	int iAlarmSchwelle2;
	int iOnDelay;
	int iAlarmDelay;
	int iWarningDelay;
	int iTcpPort;
	int iCmd;
	int iAlarms;
	int iState;
	int iExit;
}ALARMCENTER;
//--------------------------------------------------

//--------------------------------------------------
// typedefs / structs
typedef struct tagCmd
{
	char acBuf0[100];
	char acBuf1[100];
	char acBuf2[100];
	char acBuf3[100];
	char acBuf4[100];
	char acBuf5[100];
	char acBuf6[100];
	char acBuf7[100];
	char acBuf8[100];
	char acBuf9[100];
}CMD;
//--------------------------------------------------

extern ALARMCENTER tAlarmCenter ;



#define DISP   1
#define FLASH  2
#define SET    3








#endif

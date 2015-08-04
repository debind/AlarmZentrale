#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <semaphore.h>


#include <wiringPi.h>

#include <dispsw.h>

#include "types.h"
#include "main.h"

#include "configfile.h"

#define A1 6
#define A2 10
#define A3 11

#define OFF_OUTPUT          digitalWrite(A1, 0), digitalWrite(A2, 0), digitalWrite(A3, 0)

#define WARNING_OUTPUT      digitalWrite(A1, pAlarmCenter->iWarningMode & 0x01),\
	                        digitalWrite(A2, (pAlarmCenter->iWarningMode & 0x02)>>1),\
							digitalWrite(A3, (pAlarmCenter->iWarningMode & 0x04)>>2)

#define ALARM_OUTPUT        digitalWrite(A1, pAlarmCenter->iAlarmMode & 0x01),\
	                        digitalWrite(A2, (pAlarmCenter->iAlarmMode & 0x02)>>1),\
							digitalWrite(A3, (pAlarmCenter->iAlarmMode & 0x04)>>2)

#define DEACTIVATED 0
#define ACTIVATED 1
#define ALL_OFF 2
#define START_WARNING 3
#define WARNING 4
#define START_ALARM 5
#define ALARM 6

//--------------------------------------------------
// local function prototypes
void* CommandThread        (void* text);
void* InputThread          (void* text);
void  err_exit             (char *message);
void  getcmd               (char* Src, CMD* ptCmd);
void  LogData              (FILE* logfile, CMD* ptCmd);
void  WarningFkt           (ALARMCENTER* pAlarmCenter);
void  AlarmFkt             (ALARMCENTER* pAlarmCenter);
void  funcDft              (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
void  menuProcess          (void);
//--------------------------------------------------

//--------------------------------------------------
// global vars
pthread_t   threadCmd;
char		CmdBuffer[1000] = {0};
sem_t       mutex_CmdBlock; 
ALARMCENTER tAlarmCenter = {0};
//--------------------------------------------------

extern void funcMenu0 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu1 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu2 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu3 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu4 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu5 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu6 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu7 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu8 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu9 (UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu10(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu11(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu12(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu15(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu20(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu21(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);

void (*afuncptr[100])(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)={
	funcMenu0, funcMenu1, funcMenu2, funcMenu3, funcMenu4, funcMenu5, funcMenu6, funcMenu7, funcMenu8 , funcMenu9,
	funcMenu10, funcMenu11  , funcMenu12, funcDft, funcDft, funcMenu15, funcDft, funcDft, funcDft, funcDft,
	funcMenu20, funcMenu21  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft
};


int iAlarmCountDown=0;
int iWarningCountDown=0;
int iOnOffCountDown=0;


//--------------------------------------------------
// main entry
//--------------------------------------------------
int main(int argc, char * argv[])
{
	FILE* fLogFile;
	int   iCount;
	int   status ;
	int*  piStatus;
	CMD   tCmd;

	//-----------------------------------------
	// read or create new conf file
	status = configfile_Read(CONF_FILE, &tAlarmCenter);
	if (status == -1)
	{
		configfile_CreateDefault(&tAlarmCenter);
		printf("created new conf file\n");
		(void)configfile_Read(CONF_FILE, &tAlarmCenter);
	}
	//-----------------------------------------

	//-----------------------------------------
	// create semaphore
	sem_init(&mutex_CmdBlock, 0, 1);
	//-----------------------------------------

	// --------------------------------------------------------
	// run the command thread (TCP/IP)
	printf(" starting thread cmd ... ");
	if (pthread_create(&threadCmd, NULL, CommandThread, NULL))
	{
		fprintf(stderr, "pthread: pthread_create() failed.\n");
	}
	// --------------------------------------------------------

	//-----------------------------------------
	// open Logfile
	fLogFile = fopen(LOG_FILE, "a");
	//-----------------------------------------

	//--------------------------------------
	// - configure the GPIOs to use the board
	// - start display thread
	dispsw_Start();
	//--------------------------------------

	dispsw_vSetMenuValue(1,  tAlarmCenter.iOnOff);
	dispsw_vSetMenuValue(2,  tAlarmCenter.iAlarmTime);
	dispsw_vSetMenuValue(3,  tAlarmCenter.iAlarmMode);
	dispsw_vSetMenuValue(4,  tAlarmCenter.iAlarmSchwelle2);
	dispsw_vSetMenuValue(5,  tAlarmCenter.iWarningTime);
	dispsw_vSetMenuValue(6,  tAlarmCenter.iWarningMode);
	dispsw_vSetMenuValue(7,  tAlarmCenter.iAlarmSchwelle1);
	dispsw_vSetMenuValue(8,  tAlarmCenter.iOnDelay);
	dispsw_vSetMenuValue(9,  tAlarmCenter.iAlarmDelay);
	dispsw_vSetMenuValue(10, tAlarmCenter.iWarningDelay);
	dispsw_vSetMenuValue(20, tAlarmCenter.iStartHour);
	dispsw_vSetMenuValue(21, tAlarmCenter.iStopHour);

	tAlarmCenter.iState = 0;

	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	digitalWrite(A1, 0);
	digitalWrite(A2, 0);
	digitalWrite(A3, 0);

	// --------------------------------------------------------
	// welcome to the machine
	while(tAlarmCenter.iExit == 0)
	{
		//------------------------------
		// Commando Verarbeitung
		sem_wait(&mutex_CmdBlock); 
		if(tAlarmCenter.iCmd == 1)
		{
			getcmd(CmdBuffer, &tCmd);
			LogData(fLogFile, &tCmd);
			tAlarmCenter.iCmd = 0;
		
			if (strcmp(tCmd.acBuf2, "ALARM") == 0) tAlarmCenter.iAlarms++;
			if (strcmp(tCmd.acBuf2, "EXIT") == 0)  tAlarmCenter.iExit = 1;
			if (tAlarmCenter.iAlarms > 1000) tAlarmCenter.iAlarms = 1000;
		}
		sem_post(&mutex_CmdBlock);
		//------------------------------

		iCount++;
		//------------------------------
		// Alarm funktion
		if (iCount % 20 == 0)  // 1.0 second
		{
			AlarmFkt(&tAlarmCenter);

		printf("OnOff=%d A-Time=%d A-Mode=%d A-Schwelle=%d W-Time=%d W-Mode=%d W-Schwelle=%d A3A2A1=%d%d%d #=%d state=%d port=%d\n",
					    tAlarmCenter.iOnOff, tAlarmCenter.iAlarmTime, tAlarmCenter.iAlarmMode, tAlarmCenter.iAlarmSchwelle2,
				        tAlarmCenter.iWarningTime, tAlarmCenter.iWarningMode, tAlarmCenter.iAlarmSchwelle1, 
						digitalRead(A3), digitalRead(A2), digitalRead(A1), tAlarmCenter.iAlarms, tAlarmCenter.iState, tAlarmCenter.iTcpPort);
			//------------------------------
		}
		//------------------------------

		//------------------------------
		// flush logfile each 10 second
		if (iCount % 200 == 0)  // 10 second flush logfile
		{
			fflush(fLogFile);
		}
		//------------------------------

		//--------------------------------------
		// update display
		dispsw_vSetMenuValue(11, tAlarmCenter.iAlarms);
		dispsw_vSetMenuValue(12, tAlarmCenter.iState);
		dispsw_vSetMenuValue(13, iAlarmCountDown);
		dispsw_vSetMenuValue(14, iWarningCountDown);
		dispsw_vSetMenuValue(15, iOnOffCountDown);
		//--------------------------------------

		//--------------------------------------
		// process user inputs and update the
		// menu.
		dispsw_MenuUpdate();
		//--------------------------------------

		//--------------------------------------
		// process the menu
		menuProcess();
		//--------------------------------------

		usleep(MAINCYCLE*1000ul);
	}
	// --------------------------------------------------------

	//--------------------------------------
	// wait for the threads to stop
	pthread_join(threadCmd, (void*)piStatus);
	//--------------------------------------

	//--------------------------------------
	// stop the display from working. 
	// release CPU power.
	dispsw_Stop();
	//--------------------------------------

	sem_destroy(&mutex_CmdBlock);

    return status ;
}

//******************************************************************************
// alarm function
//******************************************************************************
void AlarmFkt(ALARMCENTER* pAlarmCenter)
{
	static int iCount;
	iAlarmCountDown++;
	iWarningCountDown++;
	iOnOffCountDown++;
	iCount++;

	if (pAlarmCenter->iStopHour > pAlarmCenter->iStartHour)
	{
		if ((pAlarmCenter->iHour >= pAlarmCenter->iStartHour) &&
            (pAlarmCenter->iHour < pAlarmCenter->iStopHour) )
		{
			// alarmcenter is active
		}
		else
		{
			pAlarmCenter->iAlarms = 0;
			printf("START STOP HOUR DEACTIVATED STOP>START\n");
			return;
		}
	}
	else
	{
		if ((pAlarmCenter->iHour >= pAlarmCenter->iStopHour) &&
            (pAlarmCenter->iHour < pAlarmCenter->iStartHour) )
		{
			printf("START STOP HOUR DEACTIVATED STOP<=START\n");
			pAlarmCenter->iAlarms = 0;
			return;
		}
		else
		{
			// alarmcenter is active
		}
	}


	switch(pAlarmCenter->iState)
	{
		case DEACTIVATED: // DEACTIVATED
			printf("DEACTIVATED: iOnOffCountDown=%d pAlarmCenter->iState=%d\n", iOnOffCountDown, pAlarmCenter->iState);
			if      (pAlarmCenter->iOnOff == 1) {iOnOffCountDown = 0; pAlarmCenter->iState = ACTIVATED;}
			break;
		case ACTIVATED:   // ACTIVATED
			if (iOnOffCountDown >= pAlarmCenter->iOnDelay) {pAlarmCenter->iState = ALL_OFF;}
			break;
		case ALL_OFF:     // ALL OFF
			OFF_OUTPUT;
			if      (pAlarmCenter->iOnOff == 0)                              {pAlarmCenter->iState = DEACTIVATED;}
			else if (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2) {iAlarmCountDown   = 0; pAlarmCenter->iState = START_ALARM;}
			else if (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle1) {iWarningCountDown = 0; pAlarmCenter->iState = START_WARNING;}	
			else if ( pAlarmCenter->iAlarms > 0) { if ((iCount % 10 )== 0) pAlarmCenter->iAlarms--; }
			break;
		case START_WARNING:   // START WARNING
			OFF_OUTPUT;
			if      (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2) {iAlarmCountDown   = 0; pAlarmCenter->iState = START_ALARM;}
			else if (iWarningCountDown     >= pAlarmCenter->iWarningDelay)   {iWarningCountDown = 0; pAlarmCenter->iState = WARNING;}
			break;
		case WARNING:   // WARNING
			WARNING_OUTPUT;			
			if      (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2) {iAlarmCountDown = 0; pAlarmCenter->iState = START_ALARM;}
			else if (iWarningCountDown     >= (pAlarmCenter->iWarningTime * TIME_FACTOR))    
			{
				pAlarmCenter->iAlarms -= pAlarmCenter->iAlarmSchwelle1;
				if ( pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle1)
				{
					iWarningCountDown = 0; 
				}
				else
				{
					iWarningCountDown = 0; 
					pAlarmCenter->iState = ALL_OFF;
				}
			}
			break;
		case START_ALARM:   // START ALARM
			if (iAlarmCountDown >= pAlarmCenter->iAlarmDelay) {iAlarmCountDown = 0; pAlarmCenter->iState = ALARM;}
			break;
		case ALARM:   // ALARM
			ALARM_OUTPUT;
			if (iAlarmCountDown >= (pAlarmCenter->iAlarmTime * TIME_FACTOR))    
			{
				pAlarmCenter->iAlarms -= pAlarmCenter->iAlarmSchwelle2;
				if     ( pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2)
				{
					iAlarmCountDown = 0;  
				}
				else if( pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle1)
				{
					iWarningCountDown = 0; 
					pAlarmCenter->iState = WARNING;
				}
				else
				{
					iWarningCountDown = 0; 
					pAlarmCenter->iState = ALL_OFF;
				}
			}
			break;
		default:
			break;
	}
}


//******************************************************************************
//get the whole command
//******************************************************************************
void getcmd(char* Src, CMD* ptCmd)
{
	char* pBuf;

	ptCmd->acBuf0[0] = 0;
	ptCmd->acBuf1[0] = 0;
	ptCmd->acBuf2[0] = 0;
	ptCmd->acBuf3[0] = 0;
	ptCmd->acBuf4[0] = 0;
	ptCmd->acBuf5[0] = 0;
	ptCmd->acBuf6[0] = 0;
	ptCmd->acBuf7[0] = 0;
	ptCmd->acBuf8[0] = 0;
	ptCmd->acBuf9[0] = 0;

	pBuf = strtok(Src, " ");

	if (pBuf != NULL) { strcpy(ptCmd->acBuf0, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf1, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf2, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf3, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf4, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf5, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf6, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf7, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf8, pBuf); pBuf = strtok(NULL, " "); }
	if (pBuf != NULL) { strcpy(ptCmd->acBuf9, pBuf); pBuf = strtok(NULL, " "); }

	//
	//printf( "%s %s %s %s %s %s %s\n", ptCmd->acBuf0, ptCmd->acBuf1, 
	//	                                       ptCmd->acBuf2, ptCmd->acBuf3, 
	//										   ptCmd->acBuf4, ptCmd->acBuf5, ptCmd->acBuf6);
}

//******************************************************************************
// Logdata
//******************************************************************************
void LogData(FILE* logfile, CMD* ptCmd)
{
	if (logfile == NULL) return;

	fprintf(logfile, "%s %s %s %s %s %s %s\n", ptCmd->acBuf0, ptCmd->acBuf1, 
		                                       ptCmd->acBuf2, ptCmd->acBuf3, 
											   ptCmd->acBuf4, ptCmd->acBuf5, ptCmd->acBuf6);
}



//******************************************************************************
// print error and exit 
//******************************************************************************
void err_exit(char *message)
 {
	perror(message);
	//exit(1);
 }


//******************************************************************************
// thread updating the display cyclically
//******************************************************************************
void* CommandThread(void* text)
{
	int         iStatus;
	int         nReceived;
	int         sock_fd, err;
	socklen_t   addr_size;
	struct      sockaddr_in my_addr, client_addr;
	printf(" ... done\n ");

	sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if (sock_fd == -1) {err_exit("server: Can't create new socket"); exit(0);}

	bzero(&my_addr,sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	my_addr.sin_port=htons(tAlarmCenter.iTcpPort);

	/*--- bind() ---*/
	err = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
	if (err == -1) {err_exit("server: bind() failed"); exit(0);}
	printf(" bind return: %d\n ", err);

	while (tAlarmCenter.iExit == 0)
	{

		addr_size = sizeof(struct sockaddr_in);
		nReceived = recvfrom(sock_fd, CmdBuffer, 500, 0,(struct sockaddr *)&client_addr, &addr_size);

		CmdBuffer[nReceived]=0;

		if (nReceived != 0)
		{
			sem_wait(&mutex_CmdBlock); 
			tAlarmCenter.iCmd = 1;
			while(tAlarmCenter.iCmd == 1)
			{
				sem_post(&mutex_CmdBlock);
				usleep(MAINCYCLE*1000ul);
				sem_wait(&mutex_CmdBlock); 
			}
			sem_post(&mutex_CmdBlock);
		}
	}
	close(sock_fd);

	pthread_exit(&iStatus);
}

//******************************************************************************
// Process the menu
//******************************************************************************
void menuProcess(void)
{
	static UINT8 u8MenuStatus = DISPSW_MENU_STABLE;
	static UINT8 u8MenuNumber = 200;
	static UINT8 u8MenuValue  = 0;

	if (u8MenuNumber != dispsw_u8GetMenu())
	{
		u8MenuNumber = dispsw_u8GetMenu();
		u8MenuValue  = dispsw_u8GetMenuValue(u8MenuNumber);
		afuncptr[u8MenuNumber](u8MenuNumber, u8MenuValue, DISP);
	}
	else if (u8MenuValue != dispsw_u8GetMenuValue(u8MenuNumber))
	{
		u8MenuValue = dispsw_u8GetMenuValue(u8MenuNumber);
		u8MenuStatus = DISPSW_MENU_STABLE;
		afuncptr[u8MenuNumber](u8MenuNumber, u8MenuValue, DISP);
	}
	else if ((u8MenuStatus == DISPSW_MENU_STABLE)   && (dispsw_u8GetMenuState() == DISPSW_MENU_FLASHING))
	{
    	u8MenuStatus = DISPSW_MENU_FLASHING;
		afuncptr[u8MenuNumber](u8MenuNumber, u8MenuValue, FLASH);
	}
	else if ((u8MenuStatus == DISPSW_MENU_FLASHING) && (dispsw_u8GetMenuState() == DISPSW_MENU_STABLE))
	{
    	u8MenuStatus = DISPSW_MENU_STABLE;
		afuncptr[u8MenuNumber](u8MenuNumber, u8MenuValue, SET);
	}
	else
	{
		afuncptr[u8MenuNumber](u8MenuNumber, u8MenuValue, DISP);
	}
}


//******************************************************************************
// default function pointer
//******************************************************************************
void funcDft(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)
{
	dispsw_Set(u8MenuNumber/10, u8MenuNumber%10, u8MenuValue/10, u8MenuValue%10);
}




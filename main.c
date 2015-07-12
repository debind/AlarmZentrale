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


#include <dispsw.h>

#include "types.h"
#include "main.h"

#include "configfile.h"

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

extern void funcMenu0(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu1(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu2(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu3(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu4(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu5(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu6(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu7(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu10(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu11(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);
extern void funcMenu15(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd);

void (*afuncptr[100])(UINT8 u8MenuNumber, UINT8 u8MenuValue, UINT8 u8Cmd)={
	funcMenu0, funcMenu1, funcMenu2, funcMenu3, funcMenu4, funcMenu5, funcMenu6, funcMenu7, funcDft   , funcDft,
	funcMenu10  , funcMenu11  , funcDft, funcDft, funcDft, funcMenu15, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft,
	funcDft  , funcDft  , funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft, funcDft
};


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
		configfile_CreateDefault();
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
	fLogFile = fopen(LOG_FILE, "w+");
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
	dispsw_vSetMenuValue(10, tAlarmCenter.iAlarms);

	tAlarmCenter.iState = 0;

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
		// Alarm decrement each 1 second
		if (iCount % 20 == 0)  // 0.5 second
		{
			if (tAlarmCenter.iAlarms) tAlarmCenter.iAlarms--;
		}
		//------------------------------

		//------------------------------
		// flush logfile each 5 second
		if (iCount % 200 == 0)  // 5 second flush logfile
		{
			fflush(fLogFile);
		}
		//------------------------------

		//------------------------------
		// evaluate 
		AlarmFkt(&tAlarmCenter);
		//------------------------------

		//--------------------------------------
		// update display
		dispsw_vSetMenuValue(10, tAlarmCenter.iAlarms);
		dispsw_vSetMenuValue(11, tAlarmCenter.iState);
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
	configfile_Safe(CONF_FILE, &tAlarmCenter);
	//--------------------------------------

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
	static int iCycles;

	iCycles++;
	switch(tAlarmCenter.iState)
	{
		case 0:
			iCycles = 0;
			if     (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2)
			{				
				tAlarmCenter.iState = 2;
			}
			else if(pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle1)
			{
				tAlarmCenter.iState = 1;
			}
			break;
		case 1:  // WARNING
			if     (pAlarmCenter->iAlarms >= pAlarmCenter->iAlarmSchwelle2)
			{
				tAlarmCenter.iState = 2;
				iCycles = 0;
			}
			else if (iCycles >= pAlarmCenter->iWarningTime * 20)
			{
				tAlarmCenter.iState = 0;
			}
			break;
		case 2:  // ALARM
			if (iCycles >= pAlarmCenter->iAlarmTime * 20)
			{
				if ((pAlarmCenter->iAlarms < pAlarmCenter->iAlarmSchwelle2) )
				{
					tAlarmCenter.iState = 1;
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
	exit(1);
 }


//******************************************************************************
// thread updating the display cyclically
//******************************************************************************
void* CommandThread(void* text)
{
	int         iStatus;
	char*       pBuf;
	char        Trash[200];
	int         nReceived;
	int         sock_fd, client_fd,  err, length;
	socklen_t   addr_size;
	struct      sockaddr_in my_addr, client_addr;
	printf(" ... done\n ");

	/*--- socket() ---*/
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) err_exit("server: Can't create new socket");

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(tAlarmCenter.iTcpPort);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	/*--- bind() ---*/
	err = bind(sock_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
	if (err == -1) err_exit("server: bind() failed");

	/*--- listen() ---*/
	err = listen(sock_fd, 1);
	if (err == -1) err_exit("server: listen() failed");

	/*--- accept() ---*/
	while (tAlarmCenter.iExit == 0)
	{
		/*--- accept() ---*/
		addr_size = sizeof(struct sockaddr_in);
		client_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &addr_size);
		if (client_fd == -1)  err_exit("server: accept() failed");

		pBuf      = CmdBuffer;
		nReceived = 0;
		do
		{
			length     = recv(client_fd, pBuf, 5, 0);
			nReceived += length;
			pBuf      += length;
		}
		while((CmdBuffer[nReceived-1] != 0) && (nReceived < 200));

		while (recv(client_fd, Trash, 100, 0) != 0);

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

	close(client_fd);
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




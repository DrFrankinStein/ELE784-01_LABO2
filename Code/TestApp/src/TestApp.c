/*
 * TestApp.c
 *
 *  Created on: 2016-11-17
 *      Author: AK77510
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include "Laboratoire2.h"

// Command to clear the console
#define CLR_SCREEN printf("\033[2J\033[1;1H\n")

#define TIMER_NS 5000000
#define MAX_PERIOD 12000

enum CommonList {Quit = 'q', EmptyCmd = -1};

enum OptionsMainMenuList {	IoctlMenuId = 'a'};

enum OptionsIoctlMenuList {	IoctlGet = 'a',
							IoctlSet,
							IoctlStreamOn,
							IoctlStreamOff,
							IoctlGrab,
							IoctlPanTilt,
							IoctlPanTiltReset};
int isMainSemUsed =0;
sem_t MainSem;

struct itimerspec	NewTimer, OldTimer;
timer_t				TimerID;

struct sigaction  TimerSig, old_TimerSig;           /* definition of signal action */

void SigTimerHandler (int signo)
{
	static int Period = 0;

	if (isMainSemUsed == 1)
	{
		//printf("\nTimer Handler Called!");
		sem_post(&MainSem);
	}

	Period = (Period + 1) % MAX_PERIOD;
}

int StartTimer (void)
{
	struct sigevent	 Sig;
	int				 retval = 0;

	memset (&TimerSig, 0, sizeof(struct sigaction));
	TimerSig.sa_handler = SigTimerHandler;
	if ((retval = sigaction(SIGRTMIN, &TimerSig, &old_TimerSig)) != 0)
	{
		printf("%s : Problème avec sigaction : retval = %d\n", __FUNCTION__, retval);
		return retval;
	}
	Sig.sigev_signo  = SIGRTMIN;
	Sig.sigev_notify = SIGEV_SIGNAL;
	timer_create(CLOCK_MONOTONIC, &Sig, &TimerID);
	NewTimer.it_value.tv_sec     = TIMER_NS / 1000000000L;
	NewTimer.it_value.tv_nsec	 = TIMER_NS % 1000000000L;
	NewTimer.it_interval.tv_sec  = TIMER_NS / 1000000000L;
	NewTimer.it_interval.tv_nsec = TIMER_NS % 1000000000L;
	timer_settime(TimerID, 0, &NewTimer, &OldTimer);

	return retval;
}

int StopTimer (void)
{
	int	 retval = 0;

	timer_settime(TimerID, 0, &OldTimer, NULL);
	timer_delete(TimerID);
	sigaction(SIGRTMIN, &old_TimerSig, NULL);

	return retval;
}

int Getchar_nonblock(void) {
	struct termios oldt, newt;
	int    ch=-1;

	tcgetattr( STDIN_FILENO, &oldt );
	memcpy ((void *) &newt, (void *) &oldt, sizeof(struct termios));
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK | O_NDELAY);
	ch = getchar();
   	fcntl(STDIN_FILENO, F_SETFL, 0);
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

	return ch;
}

void FlushStdIn(void)
{
	char c;
	while ((c = getchar()) != '\n' && c != EOF);
}

char WaitForKeyPressed(void)
{
	char ch = 0;
	//int tic = 0;

	//printf("\nStarting WFKP\n");

	isMainSemUsed = 1;
	while (ch < 1)
	{
		if (sem_wait(&MainSem) == 0)
		{
			//sem_wait(&MainSem);
			//printf("\nTic! (%i)", tic);
			ch = Getchar_nonblock();
			//printf("\n(%i)Value = %i",tic, ch);
			//tic++;
		}
		else
		{
			//printf("\nsemWait error: %i",errno);
		}
	}
	isMainSemUsed = 0;

	//printf("\nStoping WFKP\n");

	return ch;
}

void PressAnyKeyToContinue(void)
{
	printf("\n\nPress any key to continue...\n\n");

	WaitForKeyPressed();
}

void IoctlMenu(void)
{
	char cmd = EmptyCmd;

	while (cmd != Quit)
	{
		CLR_SCREEN;

		printf("\nIOCTL MENU :");

		//Test if we can open port
		int fd = open("/dev/Laboratoire2", O_RDONLY);

		if(fd < 0)
		{
			printf("ERROR opening the driver...(%s)\n", strerror(fd));
			PressAnyKeyToContinue();
			return;
		}
		else
		{
			close(fd);

			printf("\nPlease make a selection : \n");

			printf("\n(%c) - IoctlGet", IoctlGet );
			printf("\n(%c) - IoctlSet", IoctlSet );
			printf("\n(%c) - IoctlStreamOn", IoctlStreamOn );
			printf("\n(%c) - IoctlStreamOff", IoctlStreamOff );
			printf("\n(%c) - IoctlGrab", IoctlGrab );
			printf("\n(%c) - IoctlPanTilt", IoctlPanTilt );
			printf("\n(%c) - IoctlPanTiltReset", IoctlPanTiltReset );
			printf("\n(%c) - Quit", Quit);
			printf("\n\nMake a selection : ");

			cmd = WaitForKeyPressed();

			switch (cmd)
			{
				case IoctlGet :

				break;

				case IoctlSet :
				break;

				case IoctlStreamOn :
				break;

				case IoctlStreamOff :
				break;

				case IoctlGrab :
				break;

				case IoctlPanTilt :
				break;

				case IoctlPanTiltReset :
				break;
			}
		}
	}
}

void Init (void)
{
	sem_init(&MainSem,0,0);
	StartTimer();
}

void Stop(void)
{
	sem_destroy(&MainSem);
	StopTimer();
	printf("\n\n");
}

int main (int argc, char *argv[])
{
	char cmd = EmptyCmd;

	Init();

	while (cmd != Quit)
	{
		CLR_SCREEN;

		printf("\nWelcome to ELE784 LABO 2 Test App!");
		printf("\nPlease make a selection : \n");

		printf("\n(%c) - IOCTL", IoctlMenuId );
		printf("\n(%c) - Quit", Quit);
		printf("\n\nMake a selection : ");

		cmd = WaitForKeyPressed();

		switch (cmd)
		{
		case IoctlMenuId:
			IoctlMenu();
			break;
		}
	}

	Stop();

	return 0;
}

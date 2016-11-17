/*
 * TestApp.c
 *
 *  Created on: 2016-11-17
 *      Author: AK77510
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Laboratoire2.h"

// Command to clear the console
#define CLR_SCREEN printf("\033[2J\033[1;1H\n")

enum CommonList {Quit = -1, EmptyCmd = 0};

enum OptionsMainMenuList {	IoctlMenuId = EmptyCmd+1};

enum OptionsIoctlMenuList {	IoctlGet = EmptyCmd+1,
							IoctlSet,
							IoctlStreamOn,
							IoctlStreamOff,
							IoctlGrab,
							IoctlPanTilt,
							IoctlPanTiltReset};

void flushStdIn(void)
{
	char c;
	while ((c = getchar()) != '\n' && c != EOF);
}

void ioctlMenu(void)
{
	int cmd = EmptyCmd;

	while (cmd != Quit)
	{
		CLR_SCREEN;

		printf("\nIOCTL MENU :");

		//int fd = open()

		printf("\nPlease make a selection : \n");

		printf("\n(%i) - IoctlGet", IoctlGet );
		printf("\n(%i) - IoctlSet", IoctlSet );
		printf("\n(%i) - IoctlStreamOn", IoctlStreamOn );
		printf("\n(%i) - IoctlStreamOff", IoctlStreamOff );
		printf("\n(%i) - IoctlGrab", IoctlGrab );
		printf("\n(%i) - IoctlPanTilt", IoctlPanTilt );
		printf("\n(%i) - IoctlPanTiltReset", IoctlPanTiltReset );
		printf("\n(%i) - Quit", Quit);
		printf("\n\nMake a selection : ");

		if(scanf("%i", &cmd) != 1)
		{
			cmd = 0;
		}
		flushStdIn();

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


int main (int argc, char *argv[])
{
	int cmd = EmptyCmd;

	while (cmd != Quit)
	{
		CLR_SCREEN;

		printf("\nWelcome to ELE784 LABO 2 Test App!");
		printf("\nPlease make a selection : \n");

		printf("\n(%i) - IOCTL", IoctlMenuId );
		printf("\n(%i) - Quit", Quit);
		printf("\n\nMake a selection : ");
		if(scanf("%i", &cmd) != 1)
		{
			cmd = 0;
		}
		flushStdIn();

		switch (cmd)
		{
		case IoctlMenuId:
			ioctlMenu();
			break;
		}
	}

	return 0;
}

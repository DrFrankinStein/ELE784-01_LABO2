/*
 * TestApp.c
 *
 *  Created on: 2016-11-17
 *      Author: AK77510
 */
#include <stdio.h>
#include <unistd.h>
#include "Laboratoire2.h"

#define CLR_SCREEN printf("\033[2J\033[1;1H\n")

enum OptionsList {Quit=0, A=1,B,C,D,MAX};

void ioctlMenu(void)
{

}


int main (int argc, char *argv[])
{
	CLR_SCREEN;

	printf("\nWelcome to ELE784 LABO 2 Test App!");
	printf("\nPlease make a selection : \n");

	printf("\n(%i) - Test", A );
	printf("\n(%i) - Quit", Quit);
	printf("\n\n");

	return 0;
}

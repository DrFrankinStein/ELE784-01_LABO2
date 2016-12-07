//===================================================
//
// AUTHOR  : JULIEN LEMAY    (LEMJ16059303)
//           THIERRY DESTIN  (DEST03099102)
//
// SCHOOL  : ECOLE DE TECHNOLOGIES SUPERIEURES
//
// CLASS   : ELE784 (FALL 2016)
//
// PROJECT : LABORATOIRE2
//
// FILE    : TestApp.c
//
// DESCRIPTION : Test code for a linux 
//               camera usb driver
//
// LAST MODIFICATION : Friday, December 2nd 2016
//
//===================================================

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

enum OptionsMainMenuList { IoctlMenuId = '1'};

enum OptionsIoctlMenuList { IoctlGet = '1',
                            IoctlSet,
                            IoctlStreamOn,
                            IoctlStreamOff,
                            IoctlGrab,
                            IoctlPanTilt_UP = 'w',
                            IoctlPanTilt_DOWN = 's',
                            IoctlPanTilt_LEFT = 'a',
                            IoctlPanTilt_RIGHT = 'd',
                            IoctlPanTiltReset = 'x',
                            IoctlClearDisplay = 'c'};
int isMainSemUsed = 0;
sem_t MainSem;

struct itimerspec NewTimer, OldTimer;
timer_t TimerID;

struct sigaction  TimerSig, old_TimerSig; /* definition of signal action */

void SigTimerHandler (int signo)
{
   static int Period = 0;

   if (isMainSemUsed == 1)
   {
      sem_post(&MainSem);
   }

   Period = (Period + 1) % MAX_PERIOD;
}

int StartTimer (void)
{
   struct sigevent    Sig;
   int             retval = 0;

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
   NewTimer.it_value.tv_nsec    = TIMER_NS % 1000000000L;
   NewTimer.it_interval.tv_sec  = TIMER_NS / 1000000000L;
   NewTimer.it_interval.tv_nsec = TIMER_NS % 1000000000L;
   timer_settime(TimerID, 0, &NewTimer, &OldTimer);

   return retval;
}

int StopTimer (void)
{
   int retval = 0;

   timer_settime(TimerID, 0, &OldTimer, NULL);
   timer_delete(TimerID);
   sigaction(SIGRTMIN, &old_TimerSig, NULL);

   return retval;
}

int Getchar_nonblock(void) 
{
   struct termios oldt, newt;
   int ch=-1;

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

   isMainSemUsed = 1;
   while (ch < 1)
   {
      if (sem_wait(&MainSem) == 0)
      {
         ch = Getchar_nonblock();
      }
   }
   isMainSemUsed = 0;

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
   int eval;
   int displayRefresh = 1;

   while (cmd != Quit)
   {
	  if (displayRefresh)
	  {
      CLR_SCREEN;

      printf("\nIOCTL MENU :");
	  }
      //Test if we can open port
      int fd = open("/dev/etsele_cdev", O_RDONLY);

      if(fd < 0)
      {
         printf("ERROR opening the driver...(%s)\n", strerror(fd));
         PressAnyKeyToContinue();
         return;
      }
      else
      {
         close(fd);

         if(displayRefresh)
         {
			 printf("\nPlease make a selection : \n");

			 printf("\n(%c) - IoctlGet", IoctlGet );
			 printf("\n(%c) - IoctlSet", IoctlSet );
			 printf("\n(%c) - IoctlStreamOn", IoctlStreamOn );
			 printf("\n(%c) - IoctlStreamOff", IoctlStreamOff );
			 printf("\n(%c) - IoctlGrab", IoctlGrab );
			 printf("\n\nIoctlPanTilt :");
			 printf("\n(%c) - UP", IoctlPanTilt_UP);
			 printf("\n(%c) - DOWN", IoctlPanTilt_DOWN);
			 printf("\n(%c) - LEFT", IoctlPanTilt_LEFT);
			 printf("\n(%c) - RIGHT", IoctlPanTilt_RIGHT);
			 printf("\n(%c) - RESET", IoctlPanTiltReset);
			 printf("\n\n(%c) - Clear Display", IoctlClearDisplay);
			 printf("\n(%c) - Quit", Quit);
			 printf("\n\nMake a selection : ");
			 displayRefresh = 0;
         }

         cmd = WaitForKeyPressed();

         switch (cmd)
         {
            case IoctlGet :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_GET)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_GET...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  else
                  {
                     printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_GET...\n");
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  close(fd);
               }

            break;

            case IoctlSet :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_SET)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_SET...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  else
                  {
                     printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_SET...\n");
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  close(fd);
               }

            break;

            case IoctlStreamOn :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_STREAMON)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_STREAMON...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  else
                  {
                     //printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_STREAMON...\n");
                     //PressAnyKeyToContinue();
                     //displayRefresh = 1;
                  }
                  close(fd);
               }

            break;

            case IoctlStreamOff :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_STREAMOFF)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_STREAMOFF...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  else
                  {
                     //printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_STREAMOFF...\n");
                     //PressAnyKeyToContinue();
                     //displayRefresh = 1;
                  }
                  close(fd);
               }

            break;

            case IoctlGrab :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_GRAB)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_GRAB...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  else
                  {
                     printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_GRAB...\n");
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  close(fd);
               }

            break;

            /*case IoctlPanTilt :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_PANTILT)))
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_PANTILT...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                  }
                  else
                  {
                     printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_PANTILT...\n");
                     PressAnyKeyToContinue();
                  }
                  close(fd);
               }

            break;*/

            case IoctlPanTilt_UP:
            case IoctlPanTilt_DOWN:
            case IoctlPanTilt_LEFT:
            case IoctlPanTilt_RIGHT:
			fd = open("/dev/etsele_cdev", O_RDONLY);

				if(fd < 0)
				{
				   printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
				   PressAnyKeyToContinue();
				   displayRefresh = 1;
				}
				else
				{
					CAM_MVT dir;
					switch (cmd)
					{
					case IoctlPanTilt_UP:
						dir = CAM_UP;
						break;
					case IoctlPanTilt_DOWN:
						dir = CAM_DOWN;
						break;
					case IoctlPanTilt_LEFT:
						dir = CAM_LEFT;
						break;
					case IoctlPanTilt_RIGHT:
						dir = CAM_RIGHT;
						break;
					}

					if((eval = ioctl(fd, LAB2_IOCTL_PANTILT, &dir)) < 0)
					{
					 printf("\n\nERROR calling ioctl LAB2_IOCTL_PANTILT_RESET...(%s)\n", strerror(eval));
					 PressAnyKeyToContinue();
					 displayRefresh = 1;
					}
					/*else
					{
					 printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_PANTILT_RESET...\n");
					 PressAnyKeyToContinue();
					}*/
					close(fd);
				}
            	break;

            case IoctlPanTiltReset :
               fd = open("/dev/etsele_cdev", O_RDONLY);

               if(fd < 0)
               {
                  printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                  PressAnyKeyToContinue();
                  displayRefresh = 1;
               }
               else
               {
                  if((eval = ioctl(fd, LAB2_IOCTL_PANTILT_RESET)) < 0)
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_PANTILT_RESET...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  /*else
                  {
                     printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_PANTILT_RESET...\n");
                     PressAnyKeyToContinue();
                  }*/
                  close(fd);
               }

            break;

            case IoctlClearDisplay:
            	displayRefresh = 1;
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
      printf("\n\n(%c) - Quit", Quit);
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

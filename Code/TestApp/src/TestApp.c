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

#include <stdint.h>
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
#include "dht_data.h"

// Command to clear the console
#define CLR_SCREEN printf("\033[2J\033[1;1H\n")

#define TIMER_NS 5000000
#define MAX_PERIOD 12000

typedef struct CamInfo
{
   char     name[50];
   uint16_t min;
   uint16_t max;
   uint16_t res;
   uint16_t cur;
} CamInfo;

int getInfoTypeSize = 3, getInfoCmdSize = 4;
uint8_t getInfoType[3] = {PU_BRIGHTNESS_CONTROL, PU_CONTRAST_CONTROL, PU_GAIN_CONTROL};
uint8_t getInfoCmd[4] = {GET_MIN, GET_MAX, GET_RES, GET_CUR};

enum CommonList {Quit = 'q', EmptyCmd = -1};

enum OptionsMainMenuList { IoctlMenuId = '1',
                           TestPhoto,
                           TestCamMove,
                           TestGetSet, };

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
                            IoctlClearDisplay = 'c', };

enum OptionMoveMenuList { MovePanTilt_UP = 'w',
                          MovePanTilt_DOWN = 's',
                          MovePanTilt_LEFT = 'a',
                          MovePanTilt_RIGHT = 'd',
                          MovePanTiltReset = 'x',
                          MoveClearDisplay = 'c', };
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

int TestPictureCamSaveInTMP(void)
{
   static int index = 0;
   char name[100];
   FILE *foutput;
   unsigned char *inBuffer;
   unsigned char *finalBuf;
   int size;

   sprintf(name,"/tmp/TestPhoto%i.jpg",index++);

   inBuffer = malloc((42666) * sizeof(unsigned char));
   finalBuf = malloc((42666 * 2) * sizeof(unsigned char));

   if ((inBuffer == NULL) || (finalBuf == NULL))
   {
      free(inBuffer);
      free(finalBuf);
      return -1;
   }

   foutput = fopen(name, "wb");
   if (foutput != NULL)
   {
      int fd = open("/dev/etsele_cdev", O_RDWR);
      if(fd > 0)
      {
         ioctl(fd, LAB2_IOCTL_STREAMON);
         ioctl(fd, LAB2_IOCTL_GRAB);
         size = read(fd, inBuffer, 42666);
         ioctl(fd, LAB2_IOCTL_STREAMOFF);
         close(fd);
         if (size > 0)
         {
            memcpy(finalBuf, inBuffer, HEADERFRAME1);
            memcpy(finalBuf + HEADERFRAME1, dht_data, DHT_SIZE);
            memcpy(finalBuf + HEADERFRAME1 + DHT_SIZE,
                   inBuffer + HEADERFRAME1,
                   (size - HEADERFRAME1));
            fwrite(finalBuf, size + DHT_SIZE, 1, foutput);
         }
      }
      fclose(foutput);
      free(inBuffer);
      free(finalBuf);
      return 0;
   }
   else
   {
      free(inBuffer);
      free(finalBuf);
      return -1;
   }
}

void MoveCameraMenu(void)
{
   char cmd = EmptyCmd;
   int eval;
   int displayRefresh = 1;

   while (cmd != Quit)
   {
     if (displayRefresh)
     {
      CLR_SCREEN;

      printf("\nMOVE CAMERA MENU :");
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

          printf("\n\nMove Camera :");
          printf("\n(%c) - UP", MovePanTilt_UP);
          printf("\n(%c) - DOWN", MovePanTilt_DOWN);
          printf("\n(%c) - LEFT", MovePanTilt_LEFT);
          printf("\n(%c) - RIGHT", MovePanTilt_RIGHT);
          printf("\n(%c) - RESET", MovePanTiltReset);
          printf("\n\n(%c) - Clear Display", MoveClearDisplay);
          printf("\n(%c) - Quit", Quit);
          printf("\n\nMake a selection : ");
          displayRefresh = 0;
         }

         cmd = WaitForKeyPressed();

         switch (cmd)
         {
            case MovePanTilt_UP:
            case MovePanTilt_DOWN:
            case MovePanTilt_LEFT:
            case MovePanTilt_RIGHT:
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
                  case MovePanTilt_UP:
                     dir = CAM_UP;
                     break;
                  case MovePanTilt_DOWN:
                     dir = CAM_DOWN;
                     break;
                  case MovePanTilt_LEFT:
                     dir = CAM_LEFT;
                     break;
                  case MovePanTilt_RIGHT:
                     dir = CAM_RIGHT;
                     break;
                  }

                  if((eval = ioctl(fd, LAB2_IOCTL_PANTILT, &dir)) < 0)
                  {
                     printf("\n\nERROR calling ioctl LAB2_IOCTL_PANTILT_RESET...(%s)\n", strerror(eval));
                     PressAnyKeyToContinue();
                     displayRefresh = 1;
                  }
                  close(fd);
               }
               break;

            case MovePanTiltReset :
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
                  close(fd);
               }

            break;

            case MoveClearDisplay:
               displayRefresh = 1;
               break;
         }
      }
   }
}












void CameraOptionMenu(void)
{

   CamInfo gain = {.name = "Gain"}, brightness = {.name = "Brightness"}, contrast = {.name = "Contrast"};
   CamInfo info[3] = {brightness, contrast, gain};
   GetSetStruct getset;

   char cmd = EmptyCmd;
   int eval;
   int displayRefresh = 1;
   int i, j;

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

          fd = open("/dev/etsele_cdev", O_RDONLY);

          if(fd < 0)
          {
             printf("\n\nERROR Getting info...(%s)\n", strerror(fd));
          }
          else
          {
             for (i = 0; i < getInfoTypeSize; i++)
             for (j = 0; j < getInfoCmdSize; j++)
             {
                getset.processingUnitSelector = getInfoType[i];
                getset.requestType = getInfoCmd[j];

                if((eval = ioctl(fd, LAB2_IOCTL_GET,&getset)))
                {
                   printf("\n\nERROR calling ioctl LAB2_IOCTL_GET...(%s)\n", strerror(eval));
                }
                else
                {
                   printf("\n%X %X %X", getset.value, getset.requestType, getset.processingUnitSelector);
                }
             }


             close(fd);
          }


          printf("\n(%c) - IoctlSet", IoctlSet );
          printf("\n(%c) - IoctlStreamOn", IoctlStreamOn );
          printf("\n(%c) - IoctlStreamOff", IoctlStreamOff );
          printf("\n\n(%c) - Clear Display", IoctlClearDisplay);
          printf("\n(%c) - Quit", Quit);
          printf("\n\nMake a selection : ");
          displayRefresh = 0;
         }

         cmd = WaitForKeyPressed();

         switch (cmd)
         {
            /*case IoctlGet :
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

            break;*/

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
                     //printf("\n\nSUCCESS calling ioctl LAB2_IOCTL_GRAB...\n");
                     //PressAnyKeyToContinue();
                     //displayRefresh = 1;
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
      printf("\n(%c) - Take photo and save it to /tmp", TestPhoto );
      printf("\n(%c) - Move Camera", TestCamMove );
      printf("\n(%c) - Check or edit camera option", TestGetSet );
      printf("\n\n(%c) - Quit", Quit);
      printf("\n\nMake a selection : ");

      cmd = WaitForKeyPressed();

      switch (cmd)
      {
      case IoctlMenuId:
         IoctlMenu();
         break;
      case TestPhoto:
         TestPictureCamSaveInTMP();
         break;
      case TestCamMove:
         MoveCameraMenu();
         break;
      case TestGetSet:
         CameraOptionMenu();
         break;
      }
   }

   Stop();

   return 0;
}

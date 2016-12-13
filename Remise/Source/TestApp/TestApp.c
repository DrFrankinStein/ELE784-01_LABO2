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
// LAST MODIFICATION : Tuesday, December 13th 2016
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

// Constant for timer of 5ms
#define TIMER_NS 5000000
#define MAX_PERIOD 12000

// Structure for camera information
typedef struct CamInfo
{
   char    name[50];
   int16_t min;
   int16_t max;
   int16_t res;
   int16_t cur;
} CamInfo;

// To simplify scan of registers
int getInfoTypeSize = 3, getInfoCmdSize = 4;
uint8_t getInfoType[3] = {PU_BRIGHTNESS_CONTROL, PU_CONTRAST_CONTROL, PU_GAIN_CONTROL};
uint8_t getInfoCmd[4] = {GET_MIN, GET_MAX, GET_RES, GET_CUR};
char keyList[16];

//Lists of menus
enum CommonList {Quit = 'q', EmptyCmd = -1};

enum OptionsMainMenuList { TestPhoto = '1',
                           TestCamMove,
                           TestGetSet, };

enum OptionMoveMenuList { MovePanTilt_UP = 'w',
                          MovePanTilt_DOWN = 's',
                          MovePanTilt_LEFT = 'a',
                          MovePanTilt_RIGHT = 'd',
                          MovePanTiltReset = 'x',
                          MoveClearDisplay = 'c', };

enum OptionStatusMenuList {  OptionSet = '1',
                             OptionStreamOn,
                             OptionStreamOff,
                             OptionClearDisplay = 'c'};

// Semaphore and timer for keyboard
int isMainSemUsed = 0;
sem_t MainSem;

struct itimerspec NewTimer, OldTimer;
timer_t TimerID;

struct sigaction  TimerSig, old_TimerSig; /* definition of signal action */

// Timer handler for keyboard
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

// Get char without pressing enter
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

// Take a picture and save it in TMP
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

// Move camera menu
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

int ChoiceList(CamInfo *info, GetSetStruct* set_t)
{
   int i = 0, value = 0;
   char cmd = EmptyCmd;
   while (cmd != Quit)
   {
      CLR_SCREEN;

      printf("\nPlease select a value :\n");

      for(i = 0; i < 16; i++)
      {
         printf("\n%c - %i", keyList[i], i*(info->max - info->min)/16);
      }
      printf("\n\n(%c) - Quit", Quit);
      printf("\n\nMake a selection : ");

      cmd = WaitForKeyPressed();

      if(cmd >= '0' && cmd <= '9')
      {
         value = cmd - '0';
      }
      else if (cmd >= 'a' && cmd <= 'f')
      {
         value = 10 + cmd - 'a';
      }
      else
      {
         continue;
      }

      set_t->value = value*(info->max - info->min)/16;

      return 0;
   }
   return -1;
}

int SetMenu(CamInfo info[3], GetSetStruct* set_t)
{
   int i = 0;
   char cmd = EmptyCmd;
   while (cmd != Quit)
   {
      CLR_SCREEN;

      printf("\nSet value for :\n");

      for(i = 0; i < 2; i++)
      {
         printf("\n%c - %s", keyList[i], info[i].name);
      }
      printf("\n\n(%c) - Quit", Quit);
      printf("\n\nMake a selection : ");

      cmd = WaitForKeyPressed();

      if (cmd == keyList[0] || cmd == keyList[1])
      {
         set_t->requestType = SET_CUR;
         set_t->processingUnitSelector = getInfoType[cmd - '0'];
         if(!ChoiceList(&info[cmd - '0'], set_t))
         {
            return 0;
         }
      }
   }

   return -1;
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

      printf("\nCAMERA OPTION MENU :");
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
                   switch (getset.requestType)
                   {
                      case GET_MIN:
                         info[i].min = getset.value;
                         break;
                      case GET_MAX:
                         info[i].max = getset.value;
                         break;
                      case GET_RES:
                         info[i].res = getset.value;
                         break;
                      case GET_CUR :
                         info[i].cur = getset.value;
                         break;
                   }
                }
             }


             close(fd);
          }

          for (i = 0; i < getInfoTypeSize; i++)
          {
             printf("\n%s", info[i].name);
             printf("\nMinimum    : %i", info[i].min);
             printf("\nMaximum    : %i", info[i].max);
             printf("\nResolution : %i", info[i].res);
             printf("\nCurrent    : %i\n", info[i].cur);
          }

          printf("\n\n(%c) - IoctlSet", OptionSet );
          printf("\n(%c) - IoctlStreamOn", OptionStreamOn );
          printf("\n(%c) - IoctlStreamOff", OptionStreamOff );
          printf("\n\n(%c) - Refresh Display", OptionClearDisplay);
          printf("\n(%c) - Quit", Quit);
          printf("\n\nMake a selection : ");
          displayRefresh = 0;
         }

         cmd = WaitForKeyPressed();

         switch (cmd)
         {

            case OptionSet :
               if(!SetMenu(info, &getset))
               {
                  fd = open("/dev/etsele_cdev", O_RDONLY);

                  if(fd < 0)
                  {
                     printf("\n\nERROR opening the driver...(%s)\n", strerror(fd));
                     PressAnyKeyToContinue();
                  }
                  else
                  {
                     if((eval = ioctl(fd, LAB2_IOCTL_SET, &getset)))
                     {
                        printf("\n\nERROR calling ioctl LAB2_IOCTL_SET...(%s)\n", strerror(eval));
                        PressAnyKeyToContinue();
                     }
                     close(fd);
                  }
               }
               displayRefresh = 1;

            break;

            case OptionStreamOn :
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

            case OptionStreamOff :
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

            case OptionClearDisplay:
               displayRefresh = 1;
               break;
         }
      }
   }
}

void Init (void)
{
   int i;
   keyList[0] = '0';

   for (i = 1; i < 10; i++)
   {
      keyList[i] = '1' + i - 1;
   }

   for (i = 10; i < 16; i++)
   {
      keyList[i] = 'a' + i - 10;
   }

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

      printf("\n(%c) - Take photo and save it to /tmp", TestPhoto );
      printf("\n(%c) - Move Camera", TestCamMove );
      printf("\n(%c) - Check or edit camera option", TestGetSet );
      printf("\n\n(%c) - Quit", Quit);
      printf("\n\nMake a selection : ");

      cmd = WaitForKeyPressed();

      switch (cmd)
      {
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

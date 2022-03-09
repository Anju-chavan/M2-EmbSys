

/* Inclusion */
#include "StdTypes.h"
#include "HAL_RegisterAccess.h"
#include "HAL_GPIO.h"
#include "HAL_InterruptHandler.h"
#include "HAL_Timer0.h"
#include "Module_Keypad.h"
#include "App_Functions.h"

/*  variables defination */
/* Define Modules */
/* User buttons */
HAL_GPIO_DeviceType Start_Button  = {PORTB_BASE_ADDRESS,PIN_3,INPUT};
HAL_GPIO_DeviceType Cancel_Button = {PORTB_BASE_ADDRESS,PIN_4,INPUT};
HAL_GPIO_DeviceType PowerOFF_Button = {PORTB_BASE_ADDRESS,PIN_5,INPUT};
/* Sensors */
HAL_GPIO_DeviceType Weight_Sensor  = {PORTA_BASE_ADDRESS,PIN_3,INPUT};
HAL_GPIO_DeviceType Door_Sensor    = {PORTA_BASE_ADDRESS,PIN_2,INPUT};
/* Actuators */
HAL_GPIO_DeviceType Heater  = {PORTB_BASE_ADDRESS,PIN_7,OUTPUT};
HAL_GPIO_DeviceType Lamp    = {PORTB_BASE_ADDRESS,PIN_6,OUTPUT};
HAL_GPIO_DeviceType Motor   = {PORTC_BASE_ADDRESS,PIN_2,OUTPUT};
HAL_GPIO_DeviceType Buzzer  = {PORTC_BASE_ADDRESS,PIN_1,OUTPUT};
/* Timer Configurations */
HAL_Timer0_ConfigType Timer0_Configurations ={
          TIMER0_TIMER,
          TIMER0_16_BITS,
          TIMER0_PRESCALER_16,
          62411 /* Overflow every 25 ms */
};
/* Define variables */
APP_stateType ProgramState= APP_OFF_STATE; /* For holding the state of Application */
uint8 TimerIntCounter=0;
Time_DataType App_Time={0,0,0};
HAL_GPIO_StatusType Input_Reading;
keypad_returnDataType Keypad_Reading=KEYPAD_NOT_PRESSED;
uint8 Edit_Position=0; /* To indicate which digit is being editted now */
                       /* 0 => first  digit of hours */
                       /* 1 => second digit of hours */
                       /* 2 => first  digit of minutes */
                       /* 3 => second digit of minutes */
                       /* 4 => first  digit of seconds */
                       /* 5 => second digit of seconds */
/* Externed modules */
extern Keypad_ConfigType Keypad1;


/* function declaration */
void APP_Init(void)
{
      /* Buttons and sensors initialization */
      GPIO_DeviceInit(&Start_Button);
      GPIO_DeviceInit(&Cancel_Button);
      GPIO_DeviceInit(&PowerOFF_Button);
      GPIO_DeviceInit(&Weight_Sensor);
      GPIO_DeviceInit(&Door_Sensor);
      /* Actuators initialization */
      GPIO_DeviceInit(&Heater);
      GPIO_DeviceInit(&Lamp);
      GPIO_DeviceInit(&Motor);
      GPIO_DeviceInit(&Buzzer);
      GPIO_DeviceSet(&Buzzer);
      /* Keypad and LCD Initialization */
      Keypad_init(&Keypad1);
      Lcd_Init();
      Lcd_Cmd(_LCD_CURSOR_OFF);
      /* Timer Initialization */
      HAL_Timer0_init(&Timer0_Configurations);
      InterruptHandler_EnableInterrupt(INT_TMR0);
}

/* This function to update Time on LCD. */
void APP_Timeupdate(Time_DataType * Time_Data)
{
      Lcd_Chr(1,6,(Time_Data->hours/10)+'0');
      Lcd_Chr(1,7,(Time_Data->hours%10)+'0');
      Lcd_Chr(1,8,':');
      Lcd_Chr(1,9,(Time_Data->minutes/10)+'0');
      Lcd_Chr(1,10,(Time_Data->minutes%10)+'0');
      Lcd_Chr(1,11,':');
      Lcd_Chr(1,12,(Time_Data->seconds/10)+'0');
      Lcd_Chr(1,13,(Time_Data->seconds%10)+'0');
}

void APP_Off_Mode(void)
{
      Lcd_Cmd(_LCD_CLEAR); /* Clear LCD */
      HAL_Timer0_stop(); /* Disable Timer */
      /* Disable timer0 interrupt */
      InterruptHandler_DisableInterrupt(INT_TMR0);
      TimerIntCounter=0; /* Reset Timer counter */
      /* Enable External Interrupts to wake from sleep */
      InterruptHandler_EnableInterrupt(INT_EXT0);
      InterruptHandler_EnableInterrupt(INT_EXT1);
      InterruptHandler_EnableInterrupt(INT_EXT2);
}

void APP_WakeUp_Mode(void)
{
      /* Enable Timer */
      HAL_Timer0_start();
      /* Enable timer0 interrupt */
      InterruptHandler_EnableInterrupt(INT_TMR0);
      /* Disable External Interrupt */
      InterruptHandler_DisableInterrupt(INT_EXT0);
      InterruptHandler_DisableInterrupt(INT_EXT1);
      InterruptHandler_DisableInterrupt(INT_EXT2);
      /* Print screen layout */
      Lcd_Out(1,1,"Time:");
      Lcd_Out(2,1,"Microwave:");
      Lcd_Out(3,-3,"Door:    Food:   ");
      Lcd_Out(4,-3,"Error:");
      /* 4*16 LCD startposition of Row 3 and 4 is at col -3 */
      Lcd_Out(2,11,"Edit ");
      ProgramState = APP_EDIT_STATE; /* Edit state */
}

void APP_Edit_Mode(void)
{
      /* Update time on LCD */
      APP_Timeupdate(&App_Time);
      /* Check start button */
      GPIO_DeviceGetRead(&Start_Button,&Input_Reading);
      if(Input_Reading==LOW)   /* User pressed start*/
      {
             /* Debouncing */
             HAL_GPIO_DEBOUNCE(Start_Button.devicePortBaseAddress,Start_Button.devicePin,LOW);
             /* Start button check  */
             if(App_Time.hours == 0   &&
                App_Time.minutes == 0 &&
                App_Time.seconds == 0 ) /* Time not set */
             {
                    Lcd_Out(4,3,"TimeNotSet");
             }
             else
             {
                    GPIO_DeviceGetRead(&Weight_Sensor,&Input_Reading);
                    if(Input_Reading==HIGH) /* Food in Microwave */
                    {
                         GPIO_DeviceGetRead(&Door_Sensor,&Input_Reading);
                         if(Input_Reading==HIGH) /* Door closed */
                         {
                                 Lcd_Out(2,11,"Run  ");
                                 Lcd_Out(4,3,"          ");
                                 TimerIntCounter=0;

                                 HAL_Timer0_stop();
                                 /* Reload Timer */
                                 TMR0L=Timer0_Configurations.Timer0_Data&0x0F;
                                 TMR0H=Timer0_Configurations.Timer0_Data>>8;

                                 HAL_Timer0_start();
                                 /* Enable timer0 interrupt */
                                 InterruptHandler_EnableInterrupt(INT_TMR0);

                                 /*  Lamp is ON, Heater is ON and Motor is ON */
                                 GPIO_DeviceSet(&Lamp);
                                 GPIO_DeviceSet(&Heater);
                                 GPIO_DeviceSet(&Motor);

                                 ProgramState = APP_RUNNING_STATE;
                         }
                         else
                         {
                                 Lcd_Out(4,3,"Close Door");
                         }
                    }
                    else
                    {
                          Lcd_Out(4,3,"PutFoodIn ");
                    }
             }
      }
      /* Check Cancel button */
      GPIO_DeviceGetRead(&Cancel_Button,&Input_Reading);
      if(Input_Reading==LOW)  /* Cancel button pressed */
      {
             /* Debouncing */
             HAL_GPIO_DEBOUNCE(Cancel_Button.devicePortBaseAddress,Cancel_Button.devicePin,LOW);
             App_Time.hours = 0;
             App_Time.minutes = 0;
             App_Time.seconds = 0;
      }

      /* Check Door and  food sensors */
      GPIO_DeviceGetRead(&Weight_Sensor,&Input_Reading);
      if(Input_Reading==HIGH)  /* Food in */
      {
             Lcd_Out(3,11,"OK  ");
      }
      else
      {
             Lcd_Out(3,11,"NO  ");
      }
      GPIO_DeviceGetRead(&Door_Sensor,&Input_Reading);
      if(Input_Reading==HIGH) /* Door Closed */
      {
             Lcd_Out(3,2,"OK  ");
      }
      else
      {
             Lcd_Out(3,2,"NO ");
      }

      /* Keypad check */
      Keypad_getReading(&Keypad1,&Keypad_Reading);
      if(Keypad_Reading !=  KEYPAD_NOT_PRESSED)
      {
            if(Keypad_Reading >= '0' && Keypad_Reading <= '9')
            {
                 switch(Edit_Position)
                 {
                     case 0:
                          App_Time.hours = (App_Time.hours%10) + (Keypad_Reading-'0')*10;
                     break;
                     case 1:
                          App_Time.hours = App_Time.hours - (App_Time.hours%10) + (Keypad_Reading-'0');
                     break;
                     case 2:
                          if(Keypad_Reading < '6') /* Minutes must be less than 60 */
                          {
                              App_Time.minutes = (App_Time.minutes%10) + (Keypad_Reading-'0')*10;
                          }
                     break;
                     case 3:
                          App_Time.minutes = App_Time.minutes - (App_Time.minutes%10) + (Keypad_Reading-'0');
                     break;
                     case 4:
                          if(Keypad_Reading < '6') /* Seconds must be less than 60 */
                          {
                              App_Time.seconds = (App_Time.seconds%10) + (Keypad_Reading-'0')*10;
                          }
                     break;
                     case 5:
                          App_Time.seconds = App_Time.seconds - (App_Time.seconds%10) + (Keypad_Reading-'0');
                     break;

                 }
            }
            else if(Keypad_Reading == '*')
            {
                 if(Edit_Position == 0) Edit_Position=5;
                 else                   Edit_Position --;
            }
            else if(Keypad_Reading == '#')
            {
                 Edit_Position ++;
                 if(Edit_Position  > 5) Edit_Position=0;
            }
      }
      
      /* Check Power buttons */
      GPIO_DeviceGetRead(&PowerOFF_Button,&Input_Reading);
      if(Input_Reading== LOW) /* Power Off Button is pressed */
      {
           /* Debouncing */
           HAL_GPIO_DEBOUNCE(PowerOFF_Button.devicePortBaseAddress,PowerOFF_Button.devicePin,LOW);
           /* Reset row pins again */
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

           /* Turn OFF Buzzer */
           GPIO_DeviceSet(&Buzzer);
           ProgramState = APP_OFF_STATE;
      }
}

void APP_Run_Mode(void)
{
      if(TimerIntCounter >= 40) /* 1 sec passed */
      {
          TimerIntCounter = 0;
          if(App_Time.seconds>0)
          {
              App_Time.seconds--;
          }
          else if(App_Time.seconds==0)
          {
              App_Time.seconds=59;
              if(App_Time.minutes>0)
              {
                  App_Time.minutes--;
              }
              else
              {
                  App_Time.minutes=59;
                  if(App_Time.hours>0) App_Time.hours--;
              }
          }
                  
                  if(App_Time.seconds==0 && App_Time.minutes==0 && App_Time.hours==0)
          {
                Lcd_Out(2,11,"Done ");

                /*  Lamp is OFF, Heater is OFF and Motor is OFF */
                GPIO_DeviceClear(&Lamp);
                GPIO_DeviceClear(&Heater);
                GPIO_DeviceClear(&Motor);

                /* Turn Buzzer ON */
                GPIO_DeviceClear(&Buzzer);

                ProgramState =  APP_NOTIFICATION_STATE;
          }

      }

      /* Update time on LCD */
      APP_Timeupdate(&App_Time);

      /* Check Cancel button */
      GPIO_DeviceGetRead(&Cancel_Button,&Input_Reading);
      if(Input_Reading==LOW)  /* Cancel button pressed */
      {
             /* Debouncing */
             HAL_GPIO_DEBOUNCE(Cancel_Button.devicePortBaseAddress,Cancel_Button.devicePin,LOW);
             
             Lcd_Out(2,11,"Edit ");

             /*  Lamp is OFF, Heater is OFF and Motor is OFF */
             GPIO_DeviceClear(&Lamp);
             GPIO_DeviceClear(&Heater);
             GPIO_DeviceClear(&Motor);

             ProgramState = APP_EDIT_STATE; /* Edit state */
      }

      /* Check Door and  food sensors */
      GPIO_DeviceGetRead(&Weight_Sensor,&Input_Reading);
      if(Input_Reading==HIGH)  /* Food in */
      {
             Lcd_Out(3,11,"OK  ");
      }
      else
      {
             Lcd_Out(3,11,"NO  ");
             Lcd_Out(2,11,"Edit ");

             /*  Lamp is OFF, Heater is OFF and Motor is OFF */
             GPIO_DeviceClear(&Lamp);
             GPIO_DeviceClear(&Heater);
             GPIO_DeviceClear(&Motor);

             ProgramState = APP_EDIT_STATE; /* Edit state */
      }
      GPIO_DeviceGetRead(&Door_Sensor,&Input_Reading);
      if(Input_Reading==HIGH) /* Door Closed */
      {
             Lcd_Out(3,2,"OK  ");
      }
      else
      {
             Lcd_Out(3,2,"NO ");
             Lcd_Out(2,11,"Edit ");

             /*  Lamp is OFF, Heater is OFF and Motor is OFF */
             GPIO_DeviceClear(&Lamp);
             GPIO_DeviceClear(&Heater);
             GPIO_DeviceClear(&Motor);

             ProgramState = APP_EDIT_STATE; /* Edit state */
      }
      /* Check Power buttons */
      GPIO_DeviceGetRead(&PowerOFF_Button,&Input_Reading);
      if(Input_Reading== LOW) /* Power Off Button is pressed */
      {
           /* Debouncing */
           HAL_GPIO_DEBOUNCE(PowerOFF_Button.devicePortBaseAddress,PowerOFF_Button.devicePin,LOW);
           /* Clear Time */
           App_Time.seconds=0;
           App_Time.minutes=0;
           App_Time.hours=0;

           /*  Lamp is OFF, Heater is OFF and Motor is OFF */
           GPIO_DeviceClear(&Lamp);
           GPIO_DeviceClear(&Heater);
           GPIO_DeviceClear(&Motor);
           
           /* Reset row pins again */
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

           /* Turn OFF Buzzer */
           GPIO_DeviceSet(&Buzzer);
           ProgramState = APP_OFF_STATE;
      }
}

void APP_Notification_Mode(void)
{
      if(TimerIntCounter >= 20) /* about .5 second passed */
      {
           GPIO_DeviceToggle(&Buzzer);
           TimerIntCounter = 0;
      }
      /* Check Door and  food sensors */
      GPIO_DeviceGetRead(&Weight_Sensor,&Input_Reading);
      if(Input_Reading==HIGH)  /* Food in */
      {
             Lcd_Out(3,11,"OK  ");
      }
      else
      {
             Lcd_Out(3,11,"NO  ");

             /* Reset row pins again */
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

             /* Turn OFF Buzzer */
             GPIO_DeviceSet(&Buzzer);

             ProgramState = APP_OFF_STATE; /* Off state */
      }
      GPIO_DeviceGetRead(&Door_Sensor,&Input_Reading);
      if(Input_Reading==HIGH) /* Door Closed */
      {
             Lcd_Out(3,2,"OK  ");
      }
      else
      {
             Lcd_Out(3,2,"NO ");

             /* Reset row pins again */
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
             GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

             /* Turn OFF Buzzer */
             GPIO_DeviceSet(&Buzzer);

             ProgramState = APP_OFF_STATE; /* Off state */
      }
      /* Check Power buttons */
      GPIO_DeviceGetRead(&PowerOFF_Button,&Input_Reading);
      if(Input_Reading== LOW) /* Power Off Button is pressed */
      {
           /* Debouncing */
           HAL_GPIO_DEBOUNCE(PowerOFF_Button.devicePortBaseAddress,PowerOFF_Button.devicePin,LOW);
           /* Reset row pins again */
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

           /* Turn OFF Buzzer */
           GPIO_DeviceSet(&Buzzer);
           ProgramState = APP_OFF_STATE;
      }
	  /* Check Cancel button */
      GPIO_DeviceGetRead(&Cancel_Button,&Input_Reading);
      if(Input_Reading==LOW)  /* Cancel button pressed */
      {
             /* Debouncing */
             HAL_GPIO_DEBOUNCE(Cancel_Button.devicePortBaseAddress,Cancel_Button.devicePin,LOW);
             /* Reset row pins again */
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[0]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[1]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[2]));
           GPIO_DeviceClear(&(Keypad1.rowConfiguration[3]));

           /* Turn OFF Buzzer */
           GPIO_DeviceSet(&Buzzer);
           ProgramState = APP_OFF_STATE;
      }
}
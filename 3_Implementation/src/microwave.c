void main(){
     APP_Init();
     while(TRUE){
           switch(ProgramState)
           {
                 case APP_OFF_STATE:
                      APP_Off_Mode();
                      Sleep(); /* Go to sleep */
                 break;
                 case APP_WAKE_UP_STATE:
                      APP_WakeUp_Mode();
                 break;
                 case APP_EDIT_STATE:
                      APP_Edit_Mode();
                      Sleep(); /* Go to sleep */
                 break;
                 case APP_RUNNING_STATE:
                      APP_Run_Mode();
                      Sleep(); /* Go to sleep */
                 break;
                 case APP_NOTIFICATION_STATE:
                      APP_Notification_Mode();
                      Sleep(); /* Go to sleep */
                 break;
           }
     }
}

void interrupt(void)
{
     if(INTCON.TMR0IF==TRUE) /* Timer0 interrupt every 20 ms */
     {
           INTCON.TMR0IF=FALSE;
           TimerIntCounter++;
           HAL_RegisterWrite(TIMER0L_Reg,Timer0_Configurations.Timer0_Data&0x0F);
           HAL_RegisterWrite(TIMER0H_Reg,Timer0_Configurations.Timer0_Data>>8);
     }
     else if(INTCON.INT0IF==TRUE) /* Wake from sleep when APP_OFF_STATE */
     {
           INTCON.INT0IF=FALSE;
           HAL_GPIO_DEBOUNCE(INT0_PORT,INT0_PIN,LOW);
           ProgramState=APP_WAKE_UP_STATE;
     }
     else if(INTCON3.INT1IF==TRUE) /* Wake from sleep when APP_OFF_STATE */
     {
           INTCON3.INT1IF=FALSE;
           HAL_GPIO_DEBOUNCE(INT1_PORT,INT1_PIN,LOW);
           ProgramState=APP_WAKE_UP_STATE;
     }
     else if(INTCON3.INT2IF==TRUE) /* Wake from sleep when APP_OFF_STATE */
     {
           INTCON3.INT2IF=FALSE;
           HAL_GPIO_DEBOUNCE(INT2_PORT,INT2_PIN,LOW);
           ProgramState=APP_WAKE_UP_STATE;
     }
}
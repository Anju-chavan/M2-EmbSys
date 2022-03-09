#ifndef _APP_FUNCTIONS_H_
#define _APP_FUNCTIONS_H_

/* Inclusion */
#include "StdTypes.h"
#include "StdTypes.h"
#include "HAL.h"
#include "Module_Keypad.h"

/* Macros */
#define Sleep() _asm sleep  /* Sleep the controller */

/* Defined data types */
/*****************************************************************************/
/** Description: This is to indicate if the state of the program.           **/
/**                                                                         **/
/** Type: Enumeration.                                                      **/
/**                                                                         **/
/** Values: -  APP_OFF_STATE           => 0 -> LCD cleared and Microwave    **/
/**                                            off.                         **/
/**         -  APP_WAKE_UP_STATE       => 1 -> Transition state after wake  **/
/**                                            up just to initialize all    **/
/**                                            disabled peripherals  when   **/
/**                                            sleeping.                    **/
/**         -  APP_EDIT_STATE          => 2 -> LCD on and user edit time.   **/
/**         -  APP_RUNNING_STATE       => 3 -> Microwave is on and LCD on.  **/
/**         -  APP_NOTIFICATION_STATE  => 4 -> Microwave done and buzzer    **/
/**                                            on.                          **/
/*****************************************************************************/
typedef enum {
        APP_OFF_STATE          =0x00,
        APP_WAKE_UP_STATE      =0x01,
        APP_EDIT_STATE         =0x02,
        APP_RUNNING_STATE      =0x03,
        APP_NOTIFICATION_STATE =0x04
}APP_stateType;

/*****************************************************************************/
/** Description: This is to define Time data.                               **/
/**                                                                         **/
/** Type: Structure.                                                        **/
/**                                                                         **/
/** Elements: - seconds                                                     **/
/**           - minutes                                                     **/
/**           - hours                                                       **/
/*****************************************************************************/
typedef struct{
       sint8 seconds;
       sint8 minutes;
       sint8 hours;
}Time_DataType;

/* Function defination */
/**
  * @brief	This function to initialize all used modules in the application. 
  *	@param	None.
  *	@return	None.
  */
void APP_Init(void);

/**
  * @brief	This function to update the time on screen. 
  *	@param	None.
  *	@return	None.
  */
void APP_Timeupdate(Time_DataType * Time_Data);

/**
  * @brief	This function contains all needed processes for OFF Mode. 
  *	@param	None.
  *	@return	None.
  */
void APP_Off_Mode(void);

/**
  * @brief	This function contains all needed processes for Wakeup Mode. 
  *	@param	None.
  *	@return	None.
  */
void APP_WakeUp_Mode(void);

/**
  * @brief	This function contains all needed processes for Edit Mode. 
  *	@param	None.
  *	@return	None.
  */
void APP_Edit_Mode(void);

/**
  * @brief	This function contains all needed processes for Run Mode. 
  *	@param	None.
  *	@return	None.
  */
void APP_Run_Mode(void);

/**
  * @brief	This function contains all needed processes for Notification Mode. 
  *	@param	None.
  *	@return	None.
  */
void APP_Notification_Mode(void);

#endif /* _APP_FUNCTIONS_H_ */
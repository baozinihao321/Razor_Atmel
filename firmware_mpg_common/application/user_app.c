/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */
volatile bool G_bUserAppCharacterCorrectFlag = FALSE; /*Character Correct Flag*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

/* Existing variables (defined in other files -- should all contain "extern" */
extern u8 G_au8DebugScanfBuffer[];                     /* From debug.c */
extern u8 G_u8DebugScanfCharCount;                     /* From debug.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */
static u8 au8UserInputBuffer[USER_INPUT_BUFFER_SIZE];  /* Char buffer */

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  /*always show my name in line1*/
  u8 au8NAME[] = "A3.JohnBao";
  LCDMessage(LINE1_START_ADDR, au8NAME);
  LCDClearChars(LINE1_START_ADDR + 12, 6);  //clear LCD_BUFFER
  
  /*show my favorite lcd color*/
  LedOn(LCD_BLUE);
  LedOff(LCD_RED);
  LedOff(LCD_GREEN);
  
  
  /*clear userinputbuffer*/
  for(u8 i = 0; i < USER_INPUT_BUFFER_SIZE; i++)
  {
    au8UserInputBuffer[i] = 0;
  }
  
  
  
  
  
  
  
  
  
  
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
  /*******************************************************************************/
  // Messages Definition
  static u8 u8NumCharsMessage[] = "\n\rTotal characters received: ";
  static u8 u8BufferMessage[]   = "\n\rBuffer contents:\n\r";
  static u8 u8EmptyMessage[] = "\n\rEmpty Buffer!\n\r";
  static u8 u8CCCMessage[] = "Character count cleared!"; 
  
  
  /*************************************************************************************/
  //Variable Definition
  
  static u8 u8samplingTime = 0;                      //Take input sample interval time 20ms
  static u8 u8characterPosition = LINE2_START_ADDR;  //Set the character beginning position
  //static u8 u8TermInputBuffer[8] = {0};
  static u8 u8CorrectInputBuffer[8] = {0};           
  static u8 u8CorrectInputIndex = 0;
  
  static u32 u32TOTALNUM = 0;                        //count the total number of characters  
  u8 u8NAME[] = "John";
  u8 u8CharCount;
   
  
  /*********************************************************************************/
  /*BUTTON FUNCTION*/
  
  
  /*BUTTON0 clear the LCD_line2 and also restart the character position*/
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);              //reset the mark
    
    LCDClearChars(LINE2_START_ADDR, 20);      //clear the whole line2
    u8characterPosition = LINE2_START_ADDR;    //restart the position
  }
  
  
  /*BUTTON1 prints back the total number of characters received on the serial port */
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);            //reset the mark
    DebugLineFeed();                       //change line
    DebugPrintf(u8NumCharsMessage);        //print strings
    DebugPrintNumber(u32TOTALNUM);         //print number
    DebugLineFeed();                       //change line
  }
  
  /*BUTTON2 clears the TOTALNUM count and prints a message on the serial port*/
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);           //reset the mark
    u32TOTALNUM = 0;                      //reset the TOTALNUM count
    DebugLineFeed();                      //change line
    DebugPrintf(u8CCCMessage);            //print character counter cleared
    DebugLineFeed();                      //change line
  }
  
  
   /*BUTTON3 prints the current letter buffer that is storing my name*/
  if(WasButtonPressed(BUTTON3))
  {
    ButtonAcknowledge(BUTTON3);
    DebugLineFeed();
    DebugPrintf(u8BufferMessage);
    
    /* Make sure there's at least one character in there! */
    if(u8CorrectInputIndex == 0)
    {
      DebugPrintf(u8EmptyMessage);
    }
    else
    {
      DebugPrintf(u8CorrectInputBuffer);
    }
    
    DebugLineFeed();
  }
  
  
  
  
  
  
  
  
  
  /*JUST FOR TEST
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    
   
    u8CharCount = DebugScanf(au8UserInputBuffer);
    au8UserInputBuffer[u8CharCount] = '\0';
    DebugPrintf(u8BufferMessage);
  }
  if(u8CharCount > 0)
    {
      DebugPrintf(au8UserInputBuffer);
      DebugLineFeed();
    }
    else
    {
      DebugPrintf(u8EmptyMessage);
    }
  
  */
  
   
  
 
  
  
  
  /*******************************************************************************/ 
  /*The core loop function of the debug input characters*/
  
  if(u8samplingTime == 20)
  {
    u8samplingTime = 0;
    
    /*check tera serial debug input every 20 ms*/
    if(DebugScanf(au8UserInputBuffer))
    {
      /*Keep track of the TOTALNUM of characters that have been received*/
      u32TOTALNUM++;
      
      /*when the position back to the beginning,clear the whole line2*/
      if(u8characterPosition == LINE2_START_ADDR)
      {
        LCDClearChars(LINE2_START_ADDR, 20);   
      }
      
      /*display the character inputed on the lcd*/
      LCDMessage(u8characterPosition,au8UserInputBuffer);
      
      
      /*When the line2_screen is full, do clear Line 2*/

      if(u8characterPosition >= LINE2_END_ADDR)
      {
        u8characterPosition = LINE2_START_ADDR;     
      }
      else
      {
        u8characterPosition++;
      }
      
      /*Compare the buffer and the name regardless of the caps look*/
      if(au8UserInputBuffer[0] == u8NAME[u8CorrectInputIndex] || au8UserInputBuffer[0] == u8NAME[u8CorrectInputIndex] - 32 ||au8UserInputBuffer[0] == u8NAME[u8CorrectInputIndex] + 32)
      {
        u8CorrectInputBuffer[u8CorrectInputIndex] = au8UserInputBuffer[0];
        if(u8CorrectInputIndex == 5)
        {
          /*If all characters are correct clear the buffer and set the flag*/
          u8CorrectInputIndex = 0;
          for(u8 i = 0;i < 8;i++)
          {
            u8CorrectInputBuffer[i] = 0;
          }
          G_bUserAppCharacterCorrectFlag = TRUE;
        }
        else
        {
          u8CorrectInputIndex++;
        }
      }
      
      
    }   
  }
   
  
  
  
  
  
    
} /* end UserAppSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

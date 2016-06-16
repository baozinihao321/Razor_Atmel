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


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern AntSetupDataType G_stAntSetupData;                         /* From ant.c */

extern u32 G_u32AntApiCurrentDataTimeStamp;                       /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */

extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static u32 UserApp_u32DataMsgCount = 0;             /* Counts the number of ANT_DATA packets received */
static u32 UserApp_u32TickMsgCount = 0;             /* Counts the number of ANT_TICK packets received */

static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */

static bool flag3=0;
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
  u8 au8WelcomeMessage[] = "ANT SLAVE DEMO";
  u8 au8Instructions[] = "B0 start radio";
  
  /* Clear screen and place start messages */
#ifdef MPG1
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8WelcomeMessage); 
  LCDMessage(LINE2_START_ADDR, au8Instructions); 

  /* Start with LED0 in RED state = channel is not configured */
  LedOn(RED);
#endif /* MPG1 */
  
#ifdef MPG2
  PixelAddressType sStringLocation = {LCD_SMALL_FONT_LINE0, LCD_LEFT_MOST_COLUMN}; 
  LcdClearScreen();
  LcdLoadString(au8WelcomeMessage, LCD_FONT_SMALL, &sStringLocation); 
  sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE1;
  LcdLoadString(au8Instructions, LCD_FONT_SMALL, &sStringLocation); 
  
  /* Start with LED0 in RED state = channel is not configured */
  LedOn(RED0);
#endif /* MPG2 */
  
 /* Configure ANT for this application */
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;
  
  /* If good initialization, set state to Idle */
  if( AntChannelConfig(ANT_SLAVE) )
  {
    /* Channel is configured, so change LED to yellow */
#ifdef MPG1
    LedOff(RED);
    LedOn(YELLOW);
#endif /* MPG1 */
    
#ifdef MPG2
    LedOn(GREEN0);
#endif /* MPG2 */
    
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
#ifdef MPG1
    LedBlink(RED, LED_4HZ);
#endif /* MPG1 */
    
#ifdef MPG2
    LedBlink(RED0, LED_4HZ);
#endif /* MPG2 */

    UserApp_StateMachine = UserAppSM_Error;
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
  
  /* Look for BUTTON 0 to open channel */
  if(WasButtonPressed(BUTTON0)&&flag3==0)
  {
    /* Got the button, so complete one-time actions before next state */
    ButtonAcknowledge(BUTTON0);
    flag3=1;
    /* Queue open channel and change LED0 from yellow to blinking green to indicate channel is opening */
    AntOpenChannel();

#ifdef MPG1
    LedOff(YELLOW);
    LedBlink(GREEN, LED_2HZ);
#endif /* MPG1 */    
    
#ifdef MPG2
    LedOff(RED0);
    LedBlink(GREEN0, LED_2HZ);
#endif /* MPG2 */
    
    /* Set timer and advance states */
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp_StateMachine = UserAppSM_WaitChannelOpen;
  }
    
} /* end UserAppSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to open */
static void UserAppSM_WaitChannelOpen(void)
{
  /* Monitor the channel status to check if channel is opened */
  if(AntRadioStatus() == ANT_OPEN)
  {
#ifdef MPG1
    LedOn(GREEN);
#endif /* MPG1 */    
    
#ifdef MPG2
    LedOn(GREEN0);
#endif /* MPG2 */
    
    UserApp_StateMachine = UserAppSM_ChannelOpen;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp_u32Timeout, TIMEOUT_VALUE) )
  {
    AntCloseChannel();

#ifdef MPG1
    LedOff(GREEN);
    LedOn(YELLOW);
#endif /* MPG1 */    
    
#ifdef MPG2
    LedOn(RED0);
    LedOn(GREEN0);
#endif /* MPG2 */
    
    UserApp_StateMachine = UserAppSM_Idle;
  }
    
} /* end UserAppSM_WaitChannelOpen() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Channel is open, so monitor data */
static void UserAppSM_ChannelOpen(void)
{
  static u8 u8LastState = 0xff;
  static u8 au8TickMessage[] = "EVENT x\n\r";  /* "x" at index [6] will be replaced by the current code */
  static u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};
  bool bGotNewData;
  static bool flagforgame=0;
  static bool flagsend=0;
  static bool flag = 0;
  static u8 u8PositionOfTheMloeWillBe[]={0,6,13,19,19,13,6,0,
  13,19,6,0,13,13,13,19,
  19,0,0,6,19,13,0,6,19,
  13,19,19,19,19,13,13,0,
  13,19,19,0,0,6,6,6,
  0,0,6,6,0,6,13,19,6,
  0,0,6,13,13,13,19,6,
  0,19,13,13,6,6,13,13};
  static u8 *u8PointToPositionOfTheMloeWillBe=u8PositionOfTheMloeWillBe;
  static u8 u8CountForBit = 0;
  static u8 u8ClockDownForBegin[]="5";
  static u32 u32ClockDownForGame=0;
  static u16 u16ClockDownForBeginning=0;
  static u8 u8count=0;
  static u8 u8clockdown[]="10";
  
  static u8 u8ReceiveFromAnother[8];
  static u8 u8tempfordisplay[3];
  u16ClockDownForBeginning++;
  u8count++;
  u32ClockDownForGame++;

  /* Check for BUTTON0 to close channel */

  /* Always check for ANT messages */
  if( AntReadData() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      UserApp_u32DataMsgCount++;
      
      /* Check if the new data is the same as the old data and update as we go */
      bGotNewData = FALSE;
      for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if(G_au8AntApiCurrentData[i] != au8LastAntData[i])
        {
          bGotNewData = TRUE;
          au8LastAntData[i] = G_au8AntApiCurrentData[i];

          au8DataContent[2 * i] = HexToASCIICharUpper(G_au8AntApiCurrentData[i] / 16);
          au8DataContent[2 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentData[i] % 16); 
        }
      }
      
      if(bGotNewData)
      {


        
        AntQueueBroadcastMessage(au8TestMessage);

        /* Check for a special packet and respond */

      } /* end if(bGotNewData) */
    } /* end if(G_eAntApiCurrentMessageClass == ANT_DATA) */
    
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      UserApp_u32TickMsgCount++;

      /* Look at the TICK contents to check the event code and respond only if it's different */
      if(u8LastState != G_au8AntApiCurrentData[ANT_TICK_MSG_EVENT_CODE_INDEX])
      {
        /* The state changed so update u8LastState and queue a debug message */
        u8LastState = G_au8AntApiCurrentData[ANT_TICK_MSG_EVENT_CODE_INDEX];
        au8TickMessage[6] = HexToASCIICharUpper(u8LastState);
        DebugPrintf(au8TickMessage);

        /* Parse u8LastState to update LED status */
        switch (u8LastState)
        {
#ifdef MPG1
          /* If we are synced with a device, blue is solid */
          case RESPONSE_NO_ERROR:
          {
            LedOff(GREEN);
            LedOn(BLUE);
            flagforgame=1;
            u16ClockDownForBeginning=0;
            u8count=0;
            u32ClockDownForGame=0;
            if(flagsend==0)
            {
              flagsend=1;
              u8 begin[]={97,4,2,2,3,3,3,3};
              AntQueueBroadcastMessage(begin);
            }
            break;
          }

          /* If we are paired but missing messages, blue blinks */
          case EVENT_RX_FAIL:
          {
            LedOff(GREEN);
            LedBlink(BLUE, LED_2HZ);
            break;
          }

          /* If we drop to search, LED is green */
          case EVENT_RX_FAIL_GO_TO_SEARCH:
          {
            LedOff(BLUE);
            LedOn(GREEN);
            break;
          }
#endif /* MPG 1 */
#ifdef MPG2
          /* If we are synced with a device, blue is solid */
          case RESPONSE_NO_ERROR:
          {
            LedOff(GREEN0);
            LedOn(BLUE0);
            break;
          }

          /* If we are paired but missing messages, blue blinks */
          case EVENT_RX_FAIL:
          {
            LedOff(GREEN0);
            LedBlink(BLUE0, LED_2HZ);
            break;
          }

          /* If we drop to search, LED is green */
          case EVENT_RX_FAIL_GO_TO_SEARCH:
          {
            LedOff(BLUE0);
            LedOn(GREEN0);
            break;
          }
#endif /* MPG 2 */
          /* If the search times out, the channel should automatically close */
          case EVENT_RX_SEARCH_TIMEOUT:
          {
            DebugPrintf("Search timeout\r\n");
            break;
          }

          default:
          {
            DebugPrintf("Unexpected Event\r\n");
            break;
          }
        } /* end switch (G_au8AntApiCurrentData) */
      } /* end if (u8LastState != G_au8AntApiCurrentData[ANT_TICK_MSG_EVENT_CODE_INDEX]) */
    } /* end else if(G_eAntApiCurrentMessageClass == ANT_TICK) */
    
  } /* end AntReadData() */
  
  if(flagforgame==1)
  {
    if(u16ClockDownForBeginning<8000 && (u16ClockDownForBeginning == 1000 ||u16ClockDownForBeginning == 2000 ||
                                         u16ClockDownForBeginning == 3000 ||u16ClockDownForBeginning == 4000 ||
                                         u16ClockDownForBeginning == 5000 ||u16ClockDownForBeginning == 6000 ||
                                         u16ClockDownForBeginning == 7000 ))
    {
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR, "Clock Down:  S");
      LCDMessage(LINE1_START_ADDR+12, u8ClockDownForBegin);
      u8ClockDownForBegin[0]--;
      if(u8ClockDownForBegin[0] < '/')
      {
        flag = 1;
        u32ClockDownForGame=0;
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR, "Player1 T:");
        LCDMessage(LINE1_START_ADDR+10,u8clockdown);
        u8clockdown[0]='0';
        u8clockdown[1]='9';
        ButtonAcknowledge(BUTTON0);
        ButtonAcknowledge(BUTTON1);
        ButtonAcknowledge(BUTTON2);
        ButtonAcknowledge(BUTTON3);
      }
    }
    
    //Game Start
    if(flag == 1&&u8count == 100)
    {
      u8count = 0;
      LedOff(GREEN);
      LedOff(RED);
      LCDClearChars(LINE2_START_ADDR, 20);
      LCDMessage(LINE2_START_ADDR+*u8PointToPositionOfTheMloeWillBe, "M");
      if(WasButtonPressed(BUTTON0))
      {
         ButtonAcknowledge(BUTTON0);
         if(*u8PointToPositionOfTheMloeWillBe == 0)
         {
           u8CountForBit++;
           LedOn(GREEN);
         }
         else
         {
           LedOn(RED);
         }
         LCDClearChars(LINE2_START_ADDR,20);
         
         u8PointToPositionOfTheMloeWillBe++;
      }
      if(WasButtonPressed(BUTTON1))
      {
         ButtonAcknowledge(BUTTON1);
         if(*u8PointToPositionOfTheMloeWillBe == 6)
         {
           u8CountForBit++;
           LedOn(GREEN);
         }
         else
         {
           LedOn(RED);
         }
         LCDClearChars(LINE2_START_ADDR,20);
         
         u8PointToPositionOfTheMloeWillBe++;
      }
      if(WasButtonPressed(BUTTON2))
      {
         ButtonAcknowledge(BUTTON2);
         if(*u8PointToPositionOfTheMloeWillBe == 13)
         {
           u8CountForBit++;
           LedOn(GREEN);
         }
         else
         {
           LedOn(RED);
         }
         LCDClearChars(LINE2_START_ADDR,20);
         
         u8PointToPositionOfTheMloeWillBe++;
      }
      if(WasButtonPressed(BUTTON3))
      {
         ButtonAcknowledge(BUTTON3);
         if(*u8PointToPositionOfTheMloeWillBe == 19)
         {
           u8CountForBit++;
           LedOn(GREEN);
         }
         else
         {
           LedOn(RED);
         }
         LCDClearChars(LINE2_START_ADDR,20);
         
         u8PointToPositionOfTheMloeWillBe++;
      }
      NumberToAscii(u8CountForBit,u8tempfordisplay);
      LCDClearChars(LINE1_START_ADDR+19,2);
      LCDMessage(LINE1_START_ADDR+14,"Num:");
      LCDMessage(LINE1_START_ADDR+18,u8tempfordisplay);
      
    }
    
    //Game Clock Down
    if(flag == 1)
    {
      for(u8 i=1;i<11;i++)
      {
        if(u32ClockDownForGame == 1000*i)
        {
          LCDClearChars(LINE1_START_ADDR+10,4);
          LCDMessage(LINE1_START_ADDR+10,u8clockdown);
          u8clockdown[1]=u8clockdown[1]-1;
        }
      }
      
    }
    //Game End
    if(u32ClockDownForGame == 10000)
    {
      flag = 0;
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR, "TIME OUT!");
      LedOff(GREEN);
      LedOff(RED);
    }
    if(u32ClockDownForGame == 12000)
    {
      if((au8DataContent[0]-48)*16+au8DataContent[1]-48>u8CountForBit)
      {
        LCDMessage(LINE1_START_ADDR, "You are the loser!");
      }
      else if((au8DataContent[0]-48)*16+au8DataContent[1]-48<u8CountForBit)
      {
        LCDMessage(LINE1_START_ADDR, "You are the winner!");
      }
      
      else
      {
        LCDMessage(LINE1_START_ADDR, "Draw!               ");
      }
      LCDMessage(LINE2_START_ADDR, "Press B0 Again ");
    }
    if(WasButtonPressed(BUTTON0)&&flag==0)
      {
        ButtonAcknowledge(BUTTON0);
        LCDCommand(LCD_CLEAR_CMD);
        u8PointToPositionOfTheMloeWillBe=u8PositionOfTheMloeWillBe;
        u8CountForBit = 0;
        u8ClockDownForBegin[0]='5';
        u32ClockDownForGame=0;
        u16ClockDownForBeginning=0;
        u8count=0;
        u8clockdown[0]='1';
        u8clockdown[1]='0';
        flagforgame=0;
        flagsend=0;
        flag = 0;
        u8 begin[]={97,4,2,2,3,3,3,3};
        AntQueueBroadcastMessage(begin);
      }
    if(u32ClockDownForGame > 12000);
    //Contact to another player by ANT if the game is beginning
  }
  

  /* A slave channel can close on its own, so explicitly check channel status */
  if(AntRadioStatus() != ANT_OPEN)
  {
#ifdef MPG1
    LedBlink(GREEN, LED_2HZ);
    LedOff(BLUE);
#endif /* MPG1 */

#ifdef MPG2
    LedBlink(GREEN0, LED_2HZ);
    LedOff(BLUE0);
#endif /* MPG2 */
    u8LastState = 0xff;
    
    UserApp_u32Timeout = G_u32SystemTime1ms;
    UserApp_StateMachine = UserAppSM_WaitChannelClose;
  } /* if(AntRadioStatus() != ANT_OPEN) */
      
} /* end UserAppSM_ChannelOpen() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for channel to close */
static void UserAppSM_WaitChannelClose(void)
{
  /* Monitor the channel status to check if channel is closed */
  if(AntRadioStatus() == ANT_CLOSED)
  {
#ifdef MPG1
    LedOff(GREEN);
    LedOn(YELLOW);
#endif /* MPG1 */

#ifdef MPG2
    LedOn(GREEN0);
    LedOn(RED0);
#endif /* MPG2 */
    UserApp_StateMachine = UserAppSM_Idle;
  }
  
  /* Check for timeout */
  if( IsTimeUp(&UserApp_u32Timeout, TIMEOUT_VALUE) )
  {
#ifdef MPG1
    LedOff(GREEN);
    LedOff(YELLOW);
    LedBlink(RED, LED_4HZ);
#endif /* MPG1 */

#ifdef MPG2
    LedBlink(RED0, LED_4HZ);
    LedOff(GREEN0);
#endif /* MPG2 */
    
    UserApp_StateMachine = UserAppSM_Error;
  }
    
} /* end UserAppSM_WaitChannelClose() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{

} /* end UserAppSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/


/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_adc.h"

#include "bcomdef.h"

#include "OnBoard.h"
#include "npi.h"
#include "stdio.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#if defined ( OSAL_CBTIMER_NUM_TASKS )
  #include "osal_cbTimer.h"
#endif

#include "devinfoservice.h"
#include "simpleGATTprofile.h"
#include "Battservice.h"

/* LL */
#include "ll.h"

/* HCI */
#include "hci.h"
#include "hci_tl.h"

#if defined( CC2540_MINIDK )
  #include "simplekeys.h"
#endif


#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

/* L2CAP */
#include "l2cap.h"

/* gap */
#include "gap.h"
#include "gapgattserver.h"
#include "gapbondmgr.h"

/* GATT */
#include "gatt.h"
#include "gattservapp.h"


/* Profiles */
#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

/* Application */
#include "BLEAPP.h"

//end head 



/* This callback is triggered when a key is pressed */
void MSA_Main_KeyCallback(uint8 keys, uint8 state);


void osalInitTasks( void )
{
  uint8 taskID = 0;
  tasksEvents = (uint16 *)osal_mem_alloc( sizeof( uint16 ) * tasksCnt);
  osal_memset( tasksEvents, 0, (sizeof( uint16 ) * tasksCnt));
  /* LL Task */
  LL_Init( taskID++ );
  /* Hal Task */
  Hal_Init( taskID++ );
  /* HCI Task */
  HCI_Init( taskID++ );
	#if defined ( OSAL_CBTIMER_NUM_TASKS )
	  /* Callback Timer Tasks */
	  osal_CbTimerInit( taskID );
	  taskID += OSAL_CBTIMER_NUM_TASKS;
	#endif
  /* L2CAP Task */
  L2CAP_Init( taskID++ );

  /* GAP Task */
  GAP_Init( taskID++ );

  /* GATT Task */
  GATT_Init( taskID++ );

  /* SM Task */
  SM_Init( taskID++ );

  /* Profiles */
  GAPRole_Init( taskID++ );
  GAPBondMgr_Init( taskID++ );

  GATTServApp_Init( taskID++ );

  /* Application */
  SimpleBLEPeripheral_Init( taskID );
}


const pTaskEventHandlerFn tasksArr[] =
{
  LL_ProcessEvent,                                                  // task 0
  Hal_ProcessEvent,                                                 // task 1
  HCI_ProcessEvent,                                                 // task 2
#if defined ( OSAL_CBTIMER_NUM_TASKS )
  OSAL_CBTIMER_PROCESS_EVENT( osal_CbTimerProcessEvent ),           // task 3
#endif
  L2CAP_ProcessEvent,                                               // task 4
  GAP_ProcessEvent,                                                 // task 5
  GATT_ProcessEvent,                                                // task 6
  SM_ProcessEvent,                                                  // task 7
  GAPRole_ProcessEvent,                                             // task 8
  GAPBondMgr_ProcessEvent,                                          // task 9
  GATTServApp_ProcessEvent,                                         // task 10
  SimpleBLEPeripheral_ProcessEvent                                  // task 11
};

const uint8 tasksCnt = sizeof( tasksArr ) / sizeof( tasksArr[0] );
uint16 *tasksEvents;


int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();
  // Initialize board I/O
  InitBoard( OB_COLD );
  /* Initialze the HAL driver */
  HalDriverInit();
  /* Initialize NV system */
  osal_snv_init();
  /* Initialize LL */
  /* Initialize the operating system */
  osal_init_system();
  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();
  // Final board initialization
  InitBoard( OB_READY );
  #if defined ( POWER_SAVING )
    osal_pwrmgr_device( PWRMGR_BATTERY );
  #endif
  
  
  osal_start_system(); 
  return 0;
}
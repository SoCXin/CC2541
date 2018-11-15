/**************************************************************************************************
  Filename:       simpleBLECentral.c
  Revised:        $Date: 2011-06-20 11:57:59 -0700 (Mon, 20 Jun 2011) $
  Revision:       $Revision: 28 $

  Description:    This file contains the Simple BLE Central sample application 
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "OnBoard.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "gatt.h"
#include "ll.h"
#include "hci.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "central.h"
#include "gapbondmgr.h"
#include "simpleGATTprofile.h"
#include "simpleBLECentral.h"
#include "npi.h"
#include "stdio.h"

/*

AmoMcu 阿莫单片机   2014 12 08   

http://www.amomcu.com             社区网站
http://amomcu.taobao.com/         淘宝网店

功能： 一主三从 的主机 （后面我们也将提供连接多从机的android app， 预告哈）

操作步骤
1，主机通过 up按键 搜索从机
2，主机 center键 自动连接，可自动连接最多3个从机，
   按屏幕显示当显示 “key S1 = send data ” 时，
   按下s1 键即可，之后主机向每个从机的char1 与 char6发送轮流发送数据。
3，从机可以按下s1键 向主机的 notify char6的数据
4，主机可以按down键读取个从机的rssi 或取消。

*/

/*********************************************************************
 * MACROS
 */

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

/*********************************************************************
 * CONSTANTS
 */
// 系统定时器间隔时间
#define SBP_PERIODIC_EVT_PERIOD              400//ms   发送数据的定时间隔， 可长可短， 建议 100~10000
#define SBP_PERIODIC_EVT_AUTO_CONNECT       1600//ms   连接从机的间隔， 不能太短， 请勿任意修改

// Maximum number of scan responses
#define DEFAULT_MAX_SCAN_RES                  3//8  最大扫描从机个数， 最大为8， 更大的话没验证过

// Scan duration in ms
#define DEFAULT_SCAN_DURATION                 1000//4000  开始扫描到返回扫描结果的时间间隔， 不能太小的原因是与从机的广播间隔有关

// Discovey mode (limited, general, all)
#define DEFAULT_DISCOVERY_MODE                DEVDISC_MODE_ALL // 扫描所有从机， 当然你可以指定扫描

// TRUE to use active scan
#define DEFAULT_DISCOVERY_ACTIVE_SCAN         TRUE 

// TRUE to use white list during discovery
#define DEFAULT_DISCOVERY_WHITE_LIST          FALSE

// TRUE to use high scan duty cycle when creating link
#define DEFAULT_LINK_HIGH_DUTY_CYCLE          FALSE

// TRUE to use white list when creating link
#define DEFAULT_LINK_WHITE_LIST               FALSE

// Default RSSI polling period in ms
#define DEFAULT_RSSI_PERIOD                   1000  // 这个是使能读取rssi是每次返回结果之间的时间间隔

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         FALSE

// Minimum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_UPDATE_MIN_CONN_INTERVAL      8//  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_UPDATE_MAX_CONN_INTERVAL      8//800  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_UPDATE_SLAVE_LATENCY          0

// Supervision timeout value (units of 10ms) if automatic parameter update request is enabled
#define DEFAULT_UPDATE_CONN_TIMEOUT           100//300//600      这个是参数更新的定时时间， 不能太长， 否则影响数据发送----请多做实验再修改该值

// Default passcode
#define DEFAULT_PASSCODE                      19655 // 密码， 呵呵， 例程没用到

// Default GAP pairing mode
#define DEFAULT_PAIRING_MODE                  GAPBOND_PAIRING_MODE_WAIT_FOR_REQ

// Default MITM mode (TRUE to require passcode or OOB when pairing)
#define DEFAULT_MITM_MODE                     FALSE

// Default bonding mode, TRUE to bond
#define DEFAULT_BONDING_MODE                  TRUE

// Default GAP bonding I/O capabilities
#define DEFAULT_IO_CAPABILITIES               GAPBOND_IO_CAP_DISPLAY_ONLY

// Default service discovery timer delay in ms
#define DEFAULT_SVC_DISCOVERY_DELAY           1//1000   这个的意思是连接上从机后延时多长时间去获取从机的服务， 为了加快速度，这里我们设置为 1ms

// TRUE to filter discovery results on desired service UUID
#define DEFAULT_DEV_DISC_BY_SVC_UUID          TRUE

// Application states
enum
{
  BLE_STATE_IDLE,
  BLE_STATE_CONNECTING,
  BLE_STATE_CONNECTED,
  BLE_STATE_DISCONNECTING
};

// Discovery states
enum
{
  BLE_DISC_STATE_IDLE,                // Idle
  BLE_DISC_STATE_SVC,                 // Service discovery
  BLE_DISC_STATE_CHAR1,               // 特征值1 
  BLE_DISC_STATE_CHAR2,  
  BLE_DISC_STATE_CHAR3,  
  BLE_DISC_STATE_CHAR4,  
  BLE_DISC_STATE_CHAR5,  
  BLE_DISC_STATE_CHAR6,              // 我们的主从一体串口透传使用的  ---amomcu---
};

enum
{
  BLE_CHAR1 = 0,
  BLE_CHAR2,
  BLE_CHAR3,
  BLE_CHAR4,
  BLE_CHAR5,
  BLE_CHAR6,
  BLE_CHAR7,
};
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

// Task ID for internal task/event processing
static uint8 simpleBLETaskId;           // 本任务的 id 号

// GAP GATT Attributes
static const uint8 simpleBLEDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Central";

// Number of scan results and scan result index
static uint8 simpleBLEScanRes = 0;  // 扫描结果， 表现为个数 ，1,2,3,4 等表示扫描到几个从机
static uint8 simpleBLEScanIdx = 0;  // 表示当前处理的是哪一个从机， 0, 1, 2,3,4 等是从机的下标
static uint8 simpleBLEUpdateCnt = 0;  // 参数更新计数， 用于控制是否可以提示用户按下 S1 按键去发送数据
static uint8 simpleBLESendValue = 0;  // 用于发送的数据， 累加， 方便观察数据

// Scan result list
static gapDevRec_t simpleBLEDevList[DEFAULT_MAX_SCAN_RES];// 扫描结果列表, 内含扫描到的从机地址

// Scanning state
static uint8 simpleBLEScanning = FALSE; // 全局控制， 扫描或停止扫描

// RSSI polling state
//static uint8 simpleBLERssi = FALSE;  // 是否查询 rssi 值， 这个查询必须是在连接从机之后才可以查询

#define MAX_PERIPHERAL_NUM  DEFAULT_MAX_SCAN_RES       // 最大从机个数， 最大为 8， 最小为1， 看你需要的定义了
#define MAX_CHAR_NUM        6        // 最大去搜索的特征值个数-------切勿改动  切勿改动  切勿改动
uint8 connectedPeripheralNum = 0;   // 已连接上的从机个数 

// Connection handle of current connection 
//static uint16 simpleBLEConnHandle[MAX_PERIPHERAL_NUM] = {GAP_CONNHANDLE_INIT};
// Application state
//static uint8 simpleBLEState[MAX_PERIPHERAL_NUM] = {BLE_STATE_IDLE};

// Discovery state
//static uint8 simpleBLEDiscState[MAX_PERIPHERAL_NUM] = {BLE_DISC_STATE_IDLE};

// Discovered service start and end handle
//static uint16 simpleBLESvcStartHdl = 0;
//static uint16 simpleBLESvcEndHdl = 0;

// Discovered characteristic handle
//static uint16 simpleBLECharHdl[2] = {0};

// Value to write
//static uint8 simpleBLECharVal = 0;

// Value read/write toggle
//static bool simpleBLEDoWrite = FALSE;

// GATT read/write procedure state
//static bool simpleBLEProcedureInProgress[MAX_PERIPHERAL_NUM] = FALSE;

// amomcu 把上面屏蔽的这些参数放到一个结构体中去， 方便我们做1主3从的连接控制， 相当于每一个连接有一组自己的参数， 若是初学者， 可以好好体会一下这种写法
typedef struct
{
    // Connection handle of current connection 
    uint16 simpleBLEConnHandle;     // = {GAP_CONNHANDLE_INIT};  // 连接上后的handle值， 用于对该设备的操作， 这是个源头， 根据这个handle 可以得到很多你需要的参数
    uint8 simpleBLEDevIdx;          //设备下标 0xff 我无效值
    // Application state
    uint8 simpleBLEState;           // = {BLE_STATE_IDLE};  // 连接状态， 比如空闲、连接中、已连接、连接断开等
    // Discovery state
    uint8 simpleBLEDiscState;       // = {BLE_DISC_STATE_IDLE}; // 发现状态，

    // Discovered service start and end handle
    uint16 simpleBLESvcStartHdl;// = 0;   // 服务的开始handle
    uint16 simpleBLESvcEndHdl;// = 0;   // 服务的结束handle

    // Discovered characteristic handle
    uint16 simpleBLECharHdl[MAX_CHAR_NUM];// = {0}; // 特征值handle， 用于读写数据

    // Value to write
    uint8 simpleBLECharVal;// = 0;     // 用于写数据

    uint8 simpleBLERssi;// = FALSE;  // 是否查询 rssi 值， 这个查询必须是在连接从机之后才可以查询

    // Value read/write toggle
    bool simpleBLEDoWrite;// = FALSE;      // 是否在可写数据， 否则就可以读数据

    // GATT read/write procedure state
    bool simpleBLEProcedureInProgress;//[MAX_PERIPHERAL_NUM] = FALSE; // 是否正在操作中

    int8 rssi;//rssi 信号值

    bool updateFlag;  // 参数更新
}BLE_DEV;

// 用于 对上面的 BLE_DEV 这个结构体的初始化， 
const BLE_DEV gDevInitValue =
{
    // Connection handle of current connection 
    /*int16 simpleBLEConnHandle;     // = {GAP_CONNHANDLE_INIT};*/ GAP_CONNHANDLE_INIT,
    0xff,   //    uint8 simpleBLEDevIdx;   //设备下标 0xff 我无效值

    // Application state
    /*nt8 simpleBLEState;           // = {BLE_STATE_IDLE};*/   BLE_STATE_IDLE,
    // Discovery state
    /*nt8 simpleBLEDiscState;       // = {BLE_DISC_STATE_IDLE};*/ BLE_DISC_STATE_IDLE,

    // Discovered service start and end handle
    0,//uint16 simpleBLESvcStartHdl = 0;
    0,//uint16 simpleBLESvcEndHdl = 0;

    // Discovered characteristic handle
    {0, 0, 0, 0, 0, 0},//uint16 simpleBLECharHdl[2];// = {0};

    // Value to write
    0,//uint8 simpleBLECharVal = 0;

    FALSE,//uint8 simpleBLERssi;// = FALSE;  // 是否查询 rssi 值， 这个查询必须是在连接从机之后才可以查询

    // Value read/write toggle
    TRUE,//FALSE,//bool simpleBLEDoWrite = FALSE;

    // GATT read/write procedure state
    FALSE,//bool simpleBLEProcedureInProgress;//[MAX_PERIPHERAL_NUM] = FALSE;
    
    0,//rssi 信号值

    FALSE,//    bool updateFlag;  // 参数更新
};

static BLE_DEV gDev[MAX_PERIPHERAL_NUM] = {0};  //定义每一个从机连接数组

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLECentralProcessGATTMsg( gattMsgEvent_t *pMsg );
static void simpleBLECentralRssiCB( uint16 connHandle, int8  rssi );
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent );
static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,
                                        uint8 uiInputs, uint8 uiOutputs );
static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status );
static void simpleBLECentral_HandleKeys( uint8 shift, uint8 keys );
static void simpleBLECentral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void simpleBLEGATTDiscoveryEvent( gattMsgEvent_t *pMsg );
static void simpleBLECentralStartDiscovery( void );
static bool simpleBLEFindSvcUuid( uint16 uuid, uint8 *pData, uint8 dataLen );
static void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType );
static BLE_DEV *simpleBLECentralGetDev(uint16 connHandle);
char *bdAddr2Str ( uint8 *pAddr );
static void performPeriodicTask( void );   // amomcu 用于发送数据
static void performPeriodicTask_AutoConnect( void );  // 执行动连接

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static const gapCentralRoleCB_t simpleBLERoleCB =
{
  simpleBLECentralRssiCB,       // RSSI callback
  simpleBLECentralEventCB       // Event callback
};

// Bond Manager Callbacks
static const gapBondCBs_t simpleBLEBondCB =
{
  simpleBLECentralPasscodeCB,
  simpleBLECentralPairStateCB
};

void DLY_ms(unsigned int ms)
{                         
    unsigned int a;
    while(ms)
    {
        a=1800;
        while(a--);
        ms--;
    }
}

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLECentral_Init
 *
 * @brief   Initialization function for the Simple BLE Central App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notification).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLECentral_Init( uint8 task_id )
{
  simpleBLETaskId = task_id;

  //串口初始化
  //NPI_InitTransport(NpiSerialCallback);
  NPI_InitTransport(NULL);

  // 初始化结构体  
  for(int i = 0; i < MAX_PERIPHERAL_NUM; i++)
  {
    osal_memcpy(&(gDev[i]), &gDevInitValue, sizeof(BLE_DEV));  
  }

  // Setup Central Profile
  {
    uint8 scanRes = DEFAULT_MAX_SCAN_RES;
    GAPCentralRole_SetParameter ( GAPCENTRALROLE_MAX_SCAN_RES, sizeof( uint8 ), &scanRes );
  }
  
  // Setup GAP
  GAP_SetParamValue( TGAP_GEN_DISC_SCAN, DEFAULT_SCAN_DURATION );
  GAP_SetParamValue( TGAP_LIM_DISC_SCAN, DEFAULT_SCAN_DURATION );
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (uint8 *) simpleBLEDeviceName );

  // Setup the GAP Bond Manager
  {
    uint32 passkey = DEFAULT_PASSCODE;
    uint8 pairMode = DEFAULT_PAIRING_MODE;
    uint8 mitm = DEFAULT_MITM_MODE;
    uint8 ioCap = DEFAULT_IO_CAPABILITIES;
    uint8 bonding = DEFAULT_BONDING_MODE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof( uint8 ), &bonding );
  }  

  // Initialize GATT Client
  VOID GATT_InitClient();

  // Register to receive incoming ATT Indications/Notifications
  GATT_RegisterForInd( simpleBLETaskId );

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );         // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES ); // GATT attributes

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleBLETaskId );
  
  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );
  
  // Setup a delayed profile startup
  osal_set_event( simpleBLETaskId, START_DEVICE_EVT );
  
  NPI_PrintString("Ready to Starting\r\n");
}

/*********************************************************************
 * @fn      SimpleBLECentral_ProcessEvent
 *
 * @brief   Simple BLE Central Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLECentral_ProcessEvent( uint8 task_id, uint16 events )
{
  
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  if ( events & SYS_EVENT_MSG )// 系统事件
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLETaskId )) != NULL )
    {
      simpleBLECentral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & START_DEVICE_EVT )// 初始化后就执行这个啦
  {
    // Start the Device
    VOID GAPCentralRole_StartDevice( (gapCentralRoleCB_t *) &simpleBLERoleCB );

    // Register with bond manager after starting device
    GAPBondMgr_Register( (gapBondCBs_t *) &simpleBLEBondCB );

    LCD_WRITE_STRING("Key UP = Scan device", HAL_LCD_LINE_8);
    
    //osal_start_timerEx( simpleBLETaskId, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    return ( events ^ START_DEVICE_EVT );
  }

  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD )
    {
      osal_start_timerEx( simpleBLETaskId, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
    }

    // Perform periodic application task
    performPeriodicTask();  // amomcu 用于发送数据


    return (events ^ SBP_PERIODIC_EVT);
  }

  if ( events & SBP_AUTO_CONNECT_EVT )
  {
     // 以下都是处理用户按下 CENTER 按键后自动连接每一个扫描到的从机的控制
     if ( simpleBLEScanRes > 0 
        && simpleBLEScanIdx < simpleBLEScanRes )
     {       
        simpleBLEScanIdx++;
        if(simpleBLEScanIdx == simpleBLEScanRes)
        {
            simpleBLEScanIdx = 0;    
        }
        else
        {
            performPeriodicTask_AutoConnect();  // 执行动连接
            osal_start_timerEx( simpleBLETaskId, SBP_AUTO_CONNECT_EVT, SBP_PERIODIC_EVT_AUTO_CONNECT );

            if(simpleBLEScanIdx == 1)
                HalLedBlink (HAL_LED_2, 1, 50, 1000);//这个的意思是， 1000ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
            else if(simpleBLEScanIdx == 2)
                HalLedBlink (HAL_LED_2, 1, 50, 1000);//这个的意思是， 1000ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
        }
     }

     return (events ^ SBP_AUTO_CONNECT_EVT);
  }
  
  if ( events & START_DISCOVERY_EVT )
  {
    simpleBLECentralStartDiscovery( );//寻找特征值 handle
    
    return ( events ^ START_DISCOVERY_EVT );
  }
  
  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLECentral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLECentral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
    case KEY_CHANGE:// 按键事件
      simpleBLECentral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;

    case GATT_MSG_EVENT:// GATT 事件， 比如收到数据， 发送数据后的回应， 都在这个函数里执行
      simpleBLECentralProcessGATTMsg( (gattMsgEvent_t *) pMsg );
      break;
  }
}

/*********************************************************************
 * @fn      simpleBLECentral_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
uint8 gStatus;

// 发送特征值 的函数
/*
uint8 index,   从机标 0， 1， 2 等
uint8 charIdx,  特征值索引   CHAR1~CHAR7
uint8 value[20], 要发送的数据 buffer，
uint8 valueLen  要发送的数据长度， 要注意 CHAR1~CHAR4 都是只有一个字节的， 
                CHAR5是5个字节， CHAR6是19个字节， 那CHAR7多少， 自己找找吧  ---amomcu
*/
void OneConnetedDevice_WriteCharX(uint8 index, uint8 charIdx, uint8 value[20], uint8 valueLen)
{
    BLE_DEV *p =  &(gDev[index]);

#if 0
   NPI_PrintValue("e a=", p->simpleBLEState, 10);
   NPI_PrintValue("e b=", p->simpleBLECharHdl[charIdx], 10);
   NPI_PrintValue("e c=", p->simpleBLEProcedureInProgress, 10);
   NPI_PrintValue("e h=", p->simpleBLEConnHandle, 10);
#endif


    if ( p->simpleBLEState == BLE_STATE_CONNECTED &&
    p->simpleBLECharHdl[charIdx] != 0 &&
    /*p->simpleBLEProcedureInProgress == FALSE &&*/
    p->simpleBLEConnHandle != GAP_CONNHANDLE_INIT)
    {
        uint8 status;

        // Do a read or write as long as no other read or write is in progress
        if ( p->simpleBLEDoWrite )
        {
            // Do a write
            attWriteReq_t req;

            req.handle = p->simpleBLECharHdl[charIdx];
            req.len = valueLen;
            //req.value[0] = value;//p->simpleBLECharVal;
            osal_memcpy(req.value, value, valueLen);
            req.sig = 0;
            req.cmd = 0;

            //NPI_PrintValue("DoWrite :", p->simpleBLECharVal, 10);
            status = GATT_WriteCharValue( p->simpleBLEConnHandle, &req, simpleBLETaskId ); // 发送数据
            if(SUCCESS != status)
            {
                NPI_PrintValue("Write Error :", status, 10);
            }
        }  
        else// 以下是读取数据的 函数， 我们没用到， 你加上去就ok
        {
            // Do a read
            attReadReq_t req;
            req.handle = p->simpleBLECharHdl[charIdx];
            NPI_PrintValue("DoRead :", req.handle, 10);
            status = GATT_ReadCharValue( p->simpleBLEConnHandle, &req, simpleBLETaskId );  
        }

        p->simpleBLEProcedureInProgress = TRUE;
        
        // 请注意， 把以下这一句打开， 那么就会执行读数据了， 后面会执行写一次数据， 然后读一次数据
        //p->simpleBLEDoWrite = !(p->simpleBLEDoWrite);
    }  
}

// 按键处理
static void simpleBLECentral_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter

  if ( keys & HAL_KEY_UP )
  {
    // Start or stop discovery
    NPI_PrintString("  [KEY UP pressed!]\r\n");
    //if (connectedPeripheralNum < MAX_PERIPHERAL_NUM)
    {
      if ( !simpleBLEScanning )//不在扫描中则进行扫描
      {
        simpleBLEScanning = TRUE;
        simpleBLEScanRes = 0;        
        
        LCD_WRITE_STRING( "Discovering...", HAL_LCD_LINE_1 );
        LCD_WRITE_STRING( "", HAL_LCD_LINE_2 );
        LCD_WRITE_STRING( "", HAL_LCD_LINE_3 );
        LCD_WRITE_STRING( "", HAL_LCD_LINE_4 );
        GAPCentralRole_StartDiscovery( DEFAULT_DISCOVERY_MODE,
                                       DEFAULT_DISCOVERY_ACTIVE_SCAN,
                                       DEFAULT_DISCOVERY_WHITE_LIST );      
      }
      else // 如果在扫描中，则停止扫描， 这个时候会返回已扫描到的设备
      {
        GAPCentralRole_CancelDiscovery();
      }
    }
  }

  if ( keys & HAL_KEY_LEFT )
  {
    NPI_PrintString("  [KEY LEFT pressed!]\r\n");
    // Display discovery results
    if ( !simpleBLEScanning && simpleBLEScanRes > 0 )
    {
        // Increment index of current result (with wraparound)
        simpleBLEScanIdx++;
        if ( simpleBLEScanIdx >= simpleBLEScanRes )
        {
          simpleBLEScanIdx = 0;
        }
        // 显示当前设备号
        LCD_WRITE_STRING_VALUE( "Device", simpleBLEScanIdx + 1,
                                10, HAL_LCD_LINE_1 );
        LCD_WRITE_STRING( bdAddr2Str( simpleBLEDevList[simpleBLEScanIdx].addr ),
                          HAL_LCD_LINE_2 );
        if(gDev[simpleBLEScanIdx].simpleBLEState == BLE_STATE_CONNECTED)
        {
            LCD_WRITE_STRING( "State: Connected", HAL_LCD_LINE_3 );
        }
        else
        {
            LCD_WRITE_STRING( "State: UnConnect", HAL_LCD_LINE_3 );
        }
    }
  }

  if ( keys & HAL_KEY_RIGHT )
  {
    BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);

    NPI_PrintString("  [KEY RIGHT pressed!]\r\n");

    // Connection update
    if ( p->simpleBLEState == BLE_STATE_CONNECTED )
    {
      // 更新参数  
      GAPCentralRole_UpdateLink( p->simpleBLEConnHandle,
                                 DEFAULT_UPDATE_MIN_CONN_INTERVAL,
                                 DEFAULT_UPDATE_MAX_CONN_INTERVAL,
                                 DEFAULT_UPDATE_SLAVE_LATENCY,
                                 DEFAULT_UPDATE_CONN_TIMEOUT );
    }
  }
  
  if ( keys & HAL_KEY_CENTER )  //中间按键
  {
#if 1// 连续自动连接已扫描到的从机
    LCD_WRITE_STRING("Connecting......", HAL_LCD_LINE_8);
    simpleBLEUpdateCnt = 0;
    simpleBLESendValue = 0;
    simpleBLEScanIdx = 0;
    performPeriodicTask_AutoConnect();  // 执行动连接
    osal_start_timerEx( simpleBLETaskId, SBP_AUTO_CONNECT_EVT, SBP_PERIODIC_EVT_AUTO_CONNECT );
    HalLedBlink (HAL_LED_1, 1, 50, 1000);//这个的意思是， 1000ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
#else
    uint8 addrType;
    uint8 *peerAddr;
    
    BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);

    NPI_PrintString("  [KEY_CENTER]\r\n");
    NPI_PrintValue("Idx=", simpleBLEScanIdx, 10);
    NPI_PrintValue("State=", p->simpleBLEState, 10);
    NPI_PrintValue("num=", connectedPeripheralNum, 10);
    NPI_PrintValue("res=", simpleBLEScanRes, 10);

    
    // Connect or disconnect
    //if ( p->simpleBLEState == BLE_STATE_IDLE 
    //    || connectedPeripheralNum < MAX_PERIPHERAL_NUM)
    if ( p->simpleBLEState == BLE_STATE_IDLE )
    {
      // if there is a scan result
      if ( simpleBLEScanRes > 0 
        && simpleBLEScanIdx < simpleBLEScanRes )
      {
        // connect to current device in scan result
        peerAddr = simpleBLEDevList[simpleBLEScanIdx].addr;
        addrType = simpleBLEDevList[simpleBLEScanIdx].addrType;
      
        p->simpleBLEState = BLE_STATE_CONNECTING;
        //osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
        GAPCentralRole_EstablishLink( DEFAULT_LINK_HIGH_DUTY_CYCLE,
                                      DEFAULT_LINK_WHITE_LIST,
                                      addrType, peerAddr );
  
        LCD_WRITE_STRING( "Connecting", HAL_LCD_LINE_1 );
        LCD_WRITE_STRING( bdAddr2Str( peerAddr ), HAL_LCD_LINE_2 ); 
      }
    }
    else if ( p->simpleBLEState == BLE_STATE_CONNECTING ||
              p->simpleBLEState == BLE_STATE_CONNECTED ||
              p->simpleBLEState == BLE_STATE_DISCONNECTING)
    {
      // disconnect
      p->simpleBLEState = BLE_STATE_DISCONNECTING;

      gStatus = GAPCentralRole_TerminateLink( p->simpleBLEConnHandle );
      
      LCD_WRITE_STRING( "Disconnecting", HAL_LCD_LINE_1 ); 
    }
#endif    
  }
  
  if ( keys & HAL_KEY_DOWN )
  {    
    NPI_PrintString("  [KEY_CENTER pressed!]\r\n");

    // 注意我们下面是使能 所有设备的rssi 读取， 如果你只需要读取其中一个rssi的， 把这个循环去掉， 并指定从机号即可
    for(int i = 0; i < simpleBLEScanRes; i++)
    {
        BLE_DEV *p =  &(gDev[i]);

        // Start or cancel RSSI polling
        if ( p->simpleBLEState == BLE_STATE_CONNECTED )
        {
          if ( !p->simpleBLERssi )
          {
            p->simpleBLERssi = TRUE;
            GAPCentralRole_StartRssi( p->simpleBLEConnHandle, DEFAULT_RSSI_PERIOD );// 关键
            
            LCD_WRITE_STRING("Key DOWN = Cancel RSSI", HAL_LCD_LINE_8);
          }
          else
          {
            p->simpleBLERssi = FALSE;
            GAPCentralRole_CancelRssi( p->simpleBLEConnHandle );// 关键
            LCD_WRITE_STRING( "RSSI Cancelled", HAL_LCD_LINE_1 );

            LCD_WRITE_STRING("Key DOWN = Read RSSI", HAL_LCD_LINE_8);
          }
        }
    }
  }

  // smartRF开发板上的 S1 对应我们源码上的HAL_KEY_SW_6
  // 这里设置成发送数据功能了
  if ( keys & HAL_KEY_SW_6 )
  {
    char str[32] = {0};

    sprintf(str, "Sent CHAR to PHER.");
    HalLcdWriteString(str,  HAL_LCD_LINE_5 );

    simpleBLEScanIdx = 0;
    
    // 启动定时器发送数据
    osal_start_timerEx( simpleBLETaskId, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );// 关键

    LCD_WRITE_STRING("Key DOWN = Read RSSI", HAL_LCD_LINE_8);
  }
}

/*********************************************************************
 * @fn      simpleBLECentralProcessGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  none
 */

// 寻找当前的 connHandle (已连接的从机句柄) 对应的设备结构地址
static BLE_DEV *simpleBLECentralGetDev(uint16 connHandle)
{
    for(int i = 0; i < MAX_PERIPHERAL_NUM; i++)
    {
        if(connHandle == gDev[i].simpleBLEConnHandle)
            return &(gDev[i]);
    }
    return NULL;
}

static void simpleBLECentralProcessGATTMsg( gattMsgEvent_t *pMsg )
{
  BLE_DEV *p =  simpleBLECentralGetDev(pMsg->connHandle);

  if(p == NULL)//未找到相应的connHandle (已连接的从机句柄) 对应的设备结构地址
  {
    NPI_PrintValue("ERROR line=", __LINE__, 10);
    return;
  }
  
  if ( p->simpleBLEState != BLE_STATE_CONNECTED )
  {
    // In case a GATT message came after a connection has dropped,
    // ignore the message
    return;
  }
  
  if ( ( pMsg->method == ATT_READ_RSP ) ||
       ( ( pMsg->method == ATT_ERROR_RSP ) &&
         ( pMsg->msg.errorRsp.reqOpcode == ATT_READ_REQ ) ) )
  {
    if ( pMsg->method == ATT_ERROR_RSP )
    {
      uint8 status = pMsg->msg.errorRsp.errCode;
      
      LCD_WRITE_STRING_VALUE( "Read Error", status, 10, HAL_LCD_LINE_1 );
    }
    else
    {
      // After a successful read, display the read value
      uint8 valueRead = pMsg->msg.readRsp.value[0];

      LCD_WRITE_STRING_VALUE( "Read rsp:", valueRead, 10, HAL_LCD_LINE_1 );
    }
    p->simpleBLEProcedureInProgress = FALSE;
  }
  else if ( ( pMsg->method == ATT_WRITE_RSP ) ||
       ( ( pMsg->method == ATT_ERROR_RSP ) &&
         ( pMsg->msg.errorRsp.reqOpcode == ATT_WRITE_REQ ) ) )
  {
    
    if ( pMsg->method == ATT_ERROR_RSP == ATT_ERROR_RSP )
    {
      uint8 status = pMsg->msg.errorRsp.errCode;
      
      LCD_WRITE_STRING_VALUE( "Write Error", status, 10, HAL_LCD_LINE_1 );
    }
    else
    {
      // After a succesful write, display the value that was written and increment value
      //LCD_WRITE_STRING_VALUE( "Write sent:", (p->simpleBLECharVal)++, 10, HAL_LCD_LINE_1 );      
      LCD_WRITE_STRING_VALUE( "Write sent:", simpleBLESendValue, 10, HAL_LCD_LINE_1 );      
    }
    
    p->simpleBLEProcedureInProgress = FALSE;    

  }
  else if ( p->simpleBLEDiscState != BLE_DISC_STATE_IDLE )
  {
    simpleBLEGATTDiscoveryEvent( pMsg );
  }
  else if ( ( pMsg->method == ATT_HANDLE_VALUE_NOTI ) )   //通知
  {
    if( pMsg->msg.handleValueNoti.handle == 0x0035)     //CHAR6的通知  串口打印
    {
        //1, 收到的数据通过显示屏显示出来
        // 这里要提醒一下，从机中只有char6 才具有notify 功能， 因此， 从机发送上来的数据就是char6的， 最大19个子节
        // 另外，如果这个主机连接了几个从机， 那么也都是通过这里进来数据， 那么如何区分是什么从机发送过来的呢
        // 这个时候，我们自然是想到这个函数的形参  gattMsgEvent_t *pMsg
        // 通过查找 发现 gattMsgEvent_t -> connHandle 就是连接handle， 每个从机的连接handle是唯一的， 我们可以用这个来区分是何从机
        // 为了简单，我们假设从机发送过来的数据是字符串，这样我们直接显示就行， 无需做更多处理

        char str[32] = {0};
        char str2[32] = {0};
        // 根据 connHandle 查找对应的设备
        BLE_DEV *p =  simpleBLECentralGetDev(pMsg->connHandle);
        osal_memcpy(str, pMsg->msg.handleValueNoti.value, pMsg->msg.handleValueNoti.len );

        //1， 显示在oled显示屏上        
        sprintf(str2, "[%d]:%s", p->simpleBLEDevIdx, str);
        HalLcdWriteString(str2,  HAL_LCD_LINE_8 );
        
        //2， 这里是收到的数据通过串口透传出去
        NPI_WriteTransport(pMsg->msg.handleValueNoti.value, pMsg->msg.handleValueNoti.len ); 

        //3， 这里是收到从机数据时闪灯
        HalLedBlink (HAL_LED_3, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
    }
  }  
}

/*********************************************************************
 * @fn      simpleBLECentralRssiCB
 *
 * @brief   RSSI callback.
 *
 * @param   connHandle - connection handle
 * @param   rssi - RSSI
 *
 * @return  none
 */
static void simpleBLECentralRssiCB( uint16 connHandle, int8 rssi )
{
    if(connHandle < MAX_CHAR_NUM)
    {
        char str[32]={0};
#if 0
        gDev[connHandle].rssi = rssi;

        sprintf(str, "[0=%d][1=%d][2=%d]db", 
            (uint8) (-gDev[0].rssi),
            (uint8) (-gDev[1].rssi),
            (uint8) (-gDev[2].rssi));

        LCD_WRITE_STRING(str, HAL_LCD_LINE_2 );
#else
        gDev[connHandle].rssi = rssi;
        sprintf(str, "[%d].rssi = -%d db", connHandle, (uint8) (-rssi));
        LCD_WRITE_STRING(str, HAL_LCD_LINE_2+connHandle );
#endif        
    }
  
    //LCD_WRITE_STRING_VALUE( "RSSI -dB:", (uint8) (-rssi), 10, HAL_LCD_LINE_1 );
}

/*********************************************************************
 * @fn      simpleBLECentralEventCB
 *
 * @brief   Central event callback function.
 *
 * @param   pEvent - pointer to event structure
 *
 * @return  none
 */
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent )
{
  switch ( pEvent->gap.opcode )
  {
    case GAP_DEVICE_INIT_DONE_EVENT:  
      {
        NPI_PrintString(">>>GAP_DEVICE_INIT_DONE_EVENT\r\n");
        LCD_WRITE_STRING( "BLE Central", HAL_LCD_LINE_1 );
        LCD_WRITE_STRING( bdAddr2Str( pEvent->initDone.devAddr ),  HAL_LCD_LINE_2 );
      }
      break;

    case GAP_DEVICE_INFO_EVENT:
      {
        NPI_PrintString(">>>GAP_DEVICE_INFO_EVENT\r\n");
        // if filtering device discovery results based on service UUID
        if ( DEFAULT_DEV_DISC_BY_SVC_UUID == TRUE )
        {
          if ( simpleBLEFindSvcUuid( SIMPLEPROFILE_SERV_UUID,
                                     pEvent->deviceInfo.pEvtData,
                                     pEvent->deviceInfo.dataLen ) )
          {
            //向设备发现列表中添加一个设备
            simpleBLEAddDeviceInfo( pEvent->deviceInfo.addr, pEvent->deviceInfo.addrType );

            //显示所有广告设备地址
            //LCD_WRITE_STRING( "AdvDevice_Addr:", HAL_LCD_LINE_5 );
            //LCD_WRITE_STRING( bdAddr2Str( pEvent->deviceInfo.addr ),  HAL_LCD_LINE_6);
            //LCD_WRITE_STRING_VALUE( "RSSI -dB:", (uint8) (-1*pEvent->deviceInfo.rssi), 10, HAL_LCD_LINE_7 );
         }
        }
      }
      break;
      
    case GAP_DEVICE_DISCOVERY_EVENT: //设备发现完成
      {
        NPI_PrintString(">>>GAP_DEVICE_DISCOVERY_EVENT\r\n");
        // discovery complete
        simpleBLEScanning = FALSE;

        // if not filtering device discovery results based on service UUID
        if ( DEFAULT_DEV_DISC_BY_SVC_UUID == FALSE )
        {
          // Copy results
          simpleBLEScanRes = pEvent->discCmpl.numDevs;
          osal_memcpy( simpleBLEDevList, pEvent->discCmpl.pDevList,
                       (sizeof( gapDevRec_t ) * pEvent->discCmpl.numDevs) );
        }
        LCD_WRITE_STRING_VALUE( "Devices Found", simpleBLEScanRes,
                                10, HAL_LCD_LINE_1 );
        //NPI_PrintValue("Devices Found", simpleBLEScanRes, 10);
        //NPI_PrintString("\r\n");
        if ( simpleBLEScanRes > 0 )
        {
          char str[32] = {0};
          //HAL_DISABLE_INTERRUPTS();
          //LCD_WRITE_STRING( "<- To Select", HAL_LCD_LINE_2 );
          for(int i = 0; i<simpleBLEScanRes; i++)
          {
            //DLY_ms(50);

            sprintf(str, "[%d]=%s", i, ((uint8*) bdAddr2Str( simpleBLEDevList[i].addr )));
            
            //LCD_WRITE_STRING_VALUE("Device ", i, 10, HAL_LCD_LINE_3);
            LCD_WRITE_STRING( str, HAL_LCD_LINE_2+i );
            /*
            NPI_PrintValue( "Device", i, 10);
            NPI_PrintString(" : ");
            NPI_PrintString((uint8*) bdAddr2Str( simpleBLEDevList[i].addr ));
            NPI_PrintString("\r\n");
            */
          }
          LCD_WRITE_STRING("Key CENTER = Connect", HAL_LCD_LINE_8);
          //HAL_ENABLE_INTERRUPTS();
        }

        // initialize scan index to last device
        simpleBLEScanIdx = 0;//simpleBLEScanRes;
        //simpleBLEScanIdx = 0;        
      }
      break;

    case GAP_LINK_ESTABLISHED_EVENT://设备连接
      {
        BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);
        p->simpleBLEDevIdx = simpleBLEScanIdx;
        
        NPI_PrintValue("Idx=", simpleBLEScanIdx, 10);
        NPI_PrintString(">>>GAP_LINK_ESTABLISHED_EVENT\r\n");
        if ( pEvent->gap.hdr.status == SUCCESS )
        {  
          p->simpleBLEState = BLE_STATE_CONNECTED;

          //将handle存起来，供主机发送数据的时候使用
          p->simpleBLEConnHandle = pEvent->linkCmpl.connectionHandle;
          NPI_PrintValue("->", p->simpleBLEConnHandle, 10);
          connectedPeripheralNum++;  //每连接一个设备增加一次
          // Just for demo, we do not actually save both peripheral's characteristic values handles, 
          // after second peripheral is connected, clean simpleBLECharHdl, for second peripheral's service discovery
            
          for(int j = 0; j<MAX_CHAR_NUM; j++)
          {// 因为有些从机的handle 不知为何不返回， 但是最后的从机的handle是返回的， 每个从机特征值handle都一样， 所以我们用后面的从机handle初始化前面的设备， 都是一样的
              p->simpleBLECharHdl[j] = 0;      
          }          

          //p->simpleBLEProcedureInProgress = TRUE;    
          p->simpleBLEProcedureInProgress = FALSE;    
          
          // If service discovery not performed initiate service discovery
          // There is no characteristic records in the list, do the service discovery.
          if ( p->simpleBLECharHdl[0] == 0 )
          {
            osal_start_timerEx( simpleBLETaskId, START_DISCOVERY_EVT, DEFAULT_SVC_DISCOVERY_DELAY );
          }
          LCD_WRITE_STRING( "Connected", HAL_LCD_LINE_1 );
          LCD_WRITE_STRING( bdAddr2Str( pEvent->linkCmpl.devAddr ), HAL_LCD_LINE_2 );   
        }
        else
        {
          //p->simpleBLEState = BLE_STATE_IDLE;
          //p->simpleBLEConnHandle = GAP_CONNHANDLE_INIT;

          //p->simpleBLERssi = FALSE;
          //p->simpleBLEDiscState = BLE_DISC_STATE_IDLE;
          osal_memcpy(p, &gDevInitValue, sizeof(BLE_DEV));            
          
          LCD_WRITE_STRING( "Connect Failed", HAL_LCD_LINE_1 );
          LCD_WRITE_STRING_VALUE( "Reason:", pEvent->gap.hdr.status, 10, HAL_LCD_LINE_2 );
        }
      }
      break;

    case GAP_LINK_TERMINATED_EVENT://断开连接
      {      
        NPI_PrintString(">>>GAP_LINK_TERMINATED_EVENT\r\n");
#if 1
        BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);
#else
        BLE_DEV *p =  simpleBLECentralGetDev(pEvent->linkCmpl.connectionHandle);
        NPI_PrintValue("hdl=", pEvent->linkCmpl.connectionHandle, 10);

        if(p == NULL)//未找到相应的connHandle (已连接的从机句柄) 对应的设备结构地址
        {
          NPI_PrintValue("ERROR line=", __LINE__, 10);
          return;
        }
#endif
        //p->simpleBLEState = BLE_STATE_IDLE;
        connectedPeripheralNum--;
        // Make sure both links are disconnected.
        //p->simpleBLEConnHandle = GAP_CONNHANDLE_INIT;
        //p->simpleBLEDiscState = BLE_DISC_STATE_IDLE;
        //p->simpleBLERssi = FALSE;

        //p->simpleBLECharHdl[0] = 0;
        //p->simpleBLECharHdl[1] = 0;
        
        //p->simpleBLEProcedureInProgress = FALSE;
        osal_memcpy(p, &gDevInitValue, sizeof(BLE_DEV));  
  
        LCD_WRITE_STRING( "Disconnected", HAL_LCD_LINE_1 );
        LCD_WRITE_STRING_VALUE( "Reason:", pEvent->linkTerminate.reason,
                                10, HAL_LCD_LINE_2 );
      }
      break;

    case GAP_LINK_PARAM_UPDATE_EVENT://参数更新
      {
         NPI_PrintString(">>>GAP_LINK_PARAM_UPDATE_EVENT\r\n");
         //LCD_WRITE_STRING( "Param Update", HAL_LCD_LINE_1 );
         HalLedBlink (HAL_LED_1 | HAL_LED_2 | HAL_LED_3, 1, 50, 1000);//这个的意思是， 1000ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  

         if(++simpleBLEUpdateCnt == (simpleBLEScanRes))            
         {
             LCD_WRITE_STRING("Key S1 = Send Data", HAL_LCD_LINE_8);
         }            
      }
      break;
      
    default:
      break;
  }
}

/*********************************************************************
 * @fn      pairStateCB
 *
 * @brief   Pairing state callback.
 *
 * @return  none
 */
static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )
  {
    LCD_WRITE_STRING( "Pairing started", HAL_LCD_LINE_1 );
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )
  {
    if ( status == SUCCESS )
    {
      LCD_WRITE_STRING( "Pairing success", HAL_LCD_LINE_1 );
    }
    else
    {
      LCD_WRITE_STRING_VALUE( "Pairing fail", status, 10, HAL_LCD_LINE_1 );
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    if ( status == SUCCESS )
    {
      LCD_WRITE_STRING( "Bonding success", HAL_LCD_LINE_1 );
    }
  }
}

/*********************************************************************
 * @fn      simpleBLECentralPasscodeCB
 *
 * @brief   Passcode callback.
 *
 * @return  none
 */
static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,
                                        uint8 uiInputs, uint8 uiOutputs )
{
#if (HAL_LCD == TRUE)

  uint32  passcode;
  uint8   str[7];

  // Create random passcode
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;
  
  // Display passcode to user
  if ( uiOutputs != 0 )
  {
    LCD_WRITE_STRING( "Passcode:",  HAL_LCD_LINE_1 );
    LCD_WRITE_STRING( (char *) _ltoa(passcode, str, 10),  HAL_LCD_LINE_2 );
  }
  
  // Send passcode response
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
#endif
}

/*********************************************************************
 * @fn      simpleBLECentralStartDiscovery
 *
 * @brief   Start service discovery.
 *
 * @return  none
 */
static void simpleBLECentralStartDiscovery( void )
{
  uint8 uuid[ATT_BT_UUID_SIZE] = { LO_UINT16(SIMPLEPROFILE_SERV_UUID),
                                   HI_UINT16(SIMPLEPROFILE_SERV_UUID) };
  
  BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);

  // Initialize cached handles
  p->simpleBLESvcStartHdl = p->simpleBLESvcEndHdl = 0;
  p->simpleBLEDiscState = BLE_DISC_STATE_SVC;
                            
  GATT_DiscPrimaryServiceByUUID( p->simpleBLEConnHandle,
                                 uuid,
                                 ATT_BT_UUID_SIZE,
                                 simpleBLETaskId );
}

/*********************************************************************
 * @fn      simpleBLEGATTDiscoveryEvent
 *
 * @brief   Process GATT discovery event
 *
 * @return  none
 */

static void simpleBLEGATTDiscoveryEvent( gattMsgEvent_t *pMsg )
{
  attReadByTypeReq_t req;
  BLE_DEV *p =  simpleBLECentralGetDev(pMsg->connHandle);
  
   if ( p->simpleBLEDiscState == BLE_DISC_STATE_SVC )   
   {    
      // Service found, store handles    
      if ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP &&       
       pMsg->msg.findByTypeValueRsp.numInfo > 0 )   
      {      
        p->simpleBLESvcStartHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].handle;  
        p->simpleBLESvcEndHdl = pMsg->msg.findByTypeValueRsp.handlesInfo[0].grpEndHandle;    
      }    
  
      // If procedure complete    
      if ( ( pMsg->method == ATT_FIND_BY_TYPE_VALUE_RSP  &&           
      pMsg->hdr.status == bleProcedureComplete ) ||         
      ( pMsg->method == ATT_ERROR_RSP ) )   
      {      
        if ( p->simpleBLESvcStartHdl != 0 )      
        {        
          // Discover characteristic       
          p->simpleBLEDiscState = BLE_DISC_STATE_CHAR1;      
          req.startHandle = p->simpleBLESvcStartHdl;    
          req.endHandle = p->simpleBLESvcEndHdl;    
          req.type.len = ATT_BT_UUID_SIZE;
          req.type.uuid[0] = LO_UINT16(SIMPLEPROFILE_CHAR1_UUID);    
          req.type.uuid[1] = HI_UINT16(SIMPLEPROFILE_CHAR1_UUID);      
          GATT_ReadUsingCharUUID( p->simpleBLEConnHandle, &req, simpleBLETaskId );     
        }    
      }  
  }  
  else if ( p->simpleBLEDiscState == BLE_DISC_STATE_CHAR1 )  
  {    
    NPI_PrintValue("1 pMsg->method", pMsg->method, 10);
    NPI_PrintValue("1 numPairs", pMsg->msg.readByTypeRsp.numPairs, 10);
    // Characteristic found, store handle    
     if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&      
        pMsg->msg.readByTypeRsp.numPairs > 0 )
        {      
           p->simpleBLECharHdl[0] = BUILD_UINT16( pMsg->msg.readByTypeRsp.dataList[0],                         
           pMsg->msg.readByTypeRsp.dataList[1] );      
    
           LCD_WRITE_STRING( "CHAR 1 Found", HAL_LCD_LINE_3 );      
           p->simpleBLEProcedureInProgress = TRUE;      
       }
#if 0     
     else // pMsg->msg.readByTypeRsp.numPairs is 0.
     {
       p->simpleBLEDiscState = BLE_DISC_STATE_CHAR2;    
       req.startHandle = p->simpleBLESvcStartHdl;  
       req.endHandle = p->simpleBLESvcEndHdl;   
       req.type.len = ATT_BT_UUID_SIZE;    
       req.type.uuid[0] = LO_UINT16(SIMPLEPROFILE_CHAR2_UUID);   
       req.type.uuid[1] = HI_UINT16(SIMPLEPROFILE_CHAR2_UUID);    
       GATT_ReadUsingCharUUID( p->simpleBLEConnHandle, &req, simpleBLETaskId );  
     }
  }    
  else if ( p->simpleBLEDiscState == BLE_DISC_STATE_CHAR2 )  
  {    
    NPI_PrintValue("2 pMsg->method", pMsg->method, 10);
    NPI_PrintValue("2 numPairs", pMsg->msg.readByTypeRsp.numPairs, 10);
    // Characteristic found, store handle    
     if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&      
        pMsg->msg.readByTypeRsp.numPairs > 0 )
        {      
           p->simpleBLECharHdl[1] = BUILD_UINT16( pMsg->msg.readByTypeRsp.dataList[0],                         
           pMsg->msg.readByTypeRsp.dataList[1] );      
    
           LCD_WRITE_STRING( "CHAR 2 Found", HAL_LCD_LINE_1 );      
           p->simpleBLEProcedureInProgress = TRUE;      
       }
     else // pMsg->msg.readByTypeRsp.numPairs is 0.
     {
       p->simpleBLEDiscState = BLE_DISC_STATE_CHAR3;    
       req.startHandle = p->simpleBLESvcStartHdl;  
       req.endHandle = p->simpleBLESvcEndHdl;   
       req.type.len = ATT_BT_UUID_SIZE;    
       req.type.uuid[0] = LO_UINT16(SIMPLEPROFILE_CHAR3_UUID);   
       req.type.uuid[1] = HI_UINT16(SIMPLEPROFILE_CHAR3_UUID);    
       GATT_ReadUsingCharUUID( p->simpleBLEConnHandle, &req, simpleBLETaskId );  
     }
  }      
  else if ( p->simpleBLEDiscState == BLE_DISC_STATE_CHAR3 )  
  {    
    NPI_PrintValue("3 pMsg->method", pMsg->method, 10);
    NPI_PrintValue("3 numPairs", pMsg->msg.readByTypeRsp.numPairs, 10);
    // Characteristic found, store handle    
     if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&      
        pMsg->msg.readByTypeRsp.numPairs > 0 )
        {      
           p->simpleBLECharHdl[2] = BUILD_UINT16( pMsg->msg.readByTypeRsp.dataList[0],                         
           pMsg->msg.readByTypeRsp.dataList[1] );      
    
           LCD_WRITE_STRING( "CHAR 3 Found", HAL_LCD_LINE_1 );      
           p->simpleBLEProcedureInProgress = TRUE;      
       }
#endif     
     else // pMsg->msg.readByTypeRsp.numPairs is 0.
     {
       p->simpleBLEDiscState = BLE_DISC_STATE_CHAR6;    
       req.startHandle = p->simpleBLESvcStartHdl;  
       req.endHandle = p->simpleBLESvcEndHdl;   
       req.type.len = ATT_BT_UUID_SIZE;    
       req.type.uuid[0] = LO_UINT16(SIMPLEPROFILE_CHAR6_UUID);   
       req.type.uuid[1] = HI_UINT16(SIMPLEPROFILE_CHAR6_UUID);    
       GATT_ReadUsingCharUUID( p->simpleBLEConnHandle, &req, simpleBLETaskId );  
     }
  }        
  else if (p->simpleBLEDiscState == BLE_DISC_STATE_CHAR6)  
  {     // Characteristic found, store handle    
    if ( pMsg->method == ATT_READ_BY_TYPE_RSP &&     
    pMsg->msg.readByTypeRsp.numPairs > 0 )    
    {      
      p->simpleBLECharHdl[5] = BUILD_UINT16( pMsg->msg.readByTypeRsp.dataList[0],                                      
      pMsg->msg.readByTypeRsp.dataList[1] );     
      LCD_WRITE_STRING( "CHAR 6 Found", HAL_LCD_LINE_4 );
      p->simpleBLEProcedureInProgress = FALSE;  
    }    
    p->simpleBLEDiscState = BLE_DISC_STATE_IDLE;
  }
}
/*********************************************************************
 * @fn      simpleBLEFindSvcUuid
 *
 * @brief   Find a given UUID in an advertiser's service UUID list.
 *
 * @return  TRUE if service UUID found
 */
static bool simpleBLEFindSvcUuid( uint16 uuid, uint8 *pData, uint8 dataLen )
{
  uint8 adLen;
  uint8 adType;
  uint8 *pEnd;
  
  pEnd = pData + dataLen - 1;
  
  // While end of data not reached
  while ( pData < pEnd )
  {
    // Get length of next AD item
    adLen = *pData++;
    if ( adLen > 0 )
    {
      adType = *pData;
      
      // If AD type is for 16-bit service UUID
      if ( adType == GAP_ADTYPE_16BIT_MORE || adType == GAP_ADTYPE_16BIT_COMPLETE )
      {
        pData++;
        adLen--;
        
        // For each UUID in list
        while ( adLen >= 2 && pData < pEnd )
        {
          // Check for match
          if ( pData[0] == LO_UINT16(uuid) && pData[1] == HI_UINT16(uuid) )
          {
            // Match found
            return TRUE;
          }
          
          // Go to next
          pData += 2;
          adLen -= 2;
        }
        
        // Handle possible erroneous extra byte in UUID list
        if ( adLen == 1 )
        {
          pData++;
        }
      }
      else
      {
        // Go to next item
        pData += adLen;
      }
    }
  }
  
  // Match not found
  return FALSE;
}

/*********************************************************************
 * @fn      simpleBLEAddDeviceInfo
 *
 * @brief   Add a device to the device discovery result list
 *
 * @return  none
 */
static void simpleBLEAddDeviceInfo( uint8 *pAddr, uint8 addrType )
{
  uint8 i;
  
  // If result count not at max
  if ( simpleBLEScanRes < DEFAULT_MAX_SCAN_RES )
  {
    // Check if device is already in scan results
    for ( i = 0; i < simpleBLEScanRes; i++ )
    {
      if ( osal_memcmp( pAddr, simpleBLEDevList[i].addr , B_ADDR_LEN ) )
      {
        return;
      }
    }
    
    // Add addr to scan result list
    osal_memcpy( simpleBLEDevList[simpleBLEScanRes].addr, pAddr, B_ADDR_LEN );
    simpleBLEDevList[simpleBLEScanRes].addrType = addrType;
    
    // Increment scan result count
    simpleBLEScanRes++;
  }
}

/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;
  
  *pStr++ = '0';
  *pStr++ = 'x';
  
  // Start from end of addr
  pAddr += B_ADDR_LEN;
  
  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }
  
  *pStr = 0;
  
  return str;
}

uint8 str2hex(uint8 *str)
{
  uint8 hex[] = "0123456789ABCDEF";
  uint8 i=0,h,l;
  for(i=0;i<16;i++){
    if(hex[i]==str[0])
      h=i;
    if(hex[i]==str[1])
      l=i;
  }
  return (h*16+l);
}
/*
// 字符串对比
static uint8 str_cmp(uint8 *p1,uint8 *p2,uint8 len)
{
  uint8 i=0;
  while(i<len){
    if(p1[i]!=p2[i])
      return 0;
    i++;
  }
  return 1;
}
*/

// amomcu 用于发送数据
static void performPeriodicTask( void )
{
    static int index = 0;
    char str[32] = {0};
    //uint8 value[20];
#if 0// 用于调试
    for(int i = 0; i < simpleBLEScanRes; i++)
    {
        sprintf(str, "[%d].[%d,%d],[%08x,%d,%d]\r\n", 
            i, gDev[i].simpleBLECharHdl[0],gDev[i].simpleBLECharHdl[5],
            gDev[i].simpleBLEConnHandle,
            gDev[i].simpleBLEState,
            simpleBLEScanIdx);
        NPI_PrintString((uint8*)str);            

        /*
[0].[37,53],[00000000]
[1].[37,53],[00000001]
        */
    }
    NPI_PrintString("\r\n");
#endif    

#if 0
    // 往从机写入char1的值， 注意char1 是一个字节长度的
    value[0] = gDev[simpleBLEScanIdx].simpleBLECharVal;
    OneConnetedDevice_WriteCharX(simpleBLEScanIdx, BLE_CHAR1, value, 1);
#else
    if(index == 0)
    {
        // 往从机写入char1的值， 注意char1 是一个字节长度的
#if 0        
        OneConnetedDevice_WriteCharX(simpleBLEScanIdx, BLE_CHAR1, &(gDev[simpleBLEScanIdx].simpleBLECharVal), 1);

        // 发送的数据同样显示在显示屏上
        sprintf(str, "Char1: %d", gDev[simpleBLEScanIdx].simpleBLECharVal);
        HalLcdWriteString(str,  HAL_LCD_LINE_6 );

        gDev[simpleBLEScanIdx].simpleBLECharVal++;
#else
        OneConnetedDevice_WriteCharX(simpleBLEScanIdx, BLE_CHAR1, &simpleBLESendValue, 1);

        // 发送的数据同样显示在显示屏上
        sprintf(str, "Char1: %d", simpleBLESendValue);
        HalLcdWriteString(str,  HAL_LCD_LINE_6 );

        simpleBLESendValue++;
#endif

        HalLedBlink (HAL_LED_1, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
    }
    else if(index == 1)            
    {        
        uint32 time = osal_GetSystemClock();

        sprintf(str, "time=%06ld", time);

        // 往从机写入char6的值， 注意char6 是一个最大字节数为19的buffer， 这里为简单没有做长度判断
        OneConnetedDevice_WriteCharX(simpleBLEScanIdx, BLE_CHAR6, (uint8*)str, osal_strlen(str));

        // 发送的数据同样显示在显示屏上
        sprintf(str, "CHAR6: time=%06ld", time);
        HalLcdWriteString(str,  HAL_LCD_LINE_7 );
        
        // 更换成下一个从机， 注意我们都是先发 
        simpleBLEScanIdx++;
        simpleBLEScanIdx %= simpleBLEScanRes;

        HalLedBlink (HAL_LED_2, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
    }

    index++;
    index %=2;
#endif    
}

// 执行动连接
static void performPeriodicTask_AutoConnect( void )  // 执行动连接
{
    uint8 addrType;
    uint8 *peerAddr;
    
    BLE_DEV *p =  &(gDev[simpleBLEScanIdx]);

    //NPI_PrintString("  [KEY_CENTER]\r\n");
    NPI_PrintValue("Idx=", simpleBLEScanIdx, 10);
    //NPI_PrintValue("State=", p->simpleBLEState, 10);
    //NPI_PrintValue("num=", connectedPeripheralNum, 10);
    //NPI_PrintValue("res=", simpleBLEScanRes, 10);

    
    // Connect or disconnect
    //if ( p->simpleBLEState == BLE_STATE_IDLE 
    //    || connectedPeripheralNum < MAX_PERIPHERAL_NUM)
    if ( p->simpleBLEState == BLE_STATE_IDLE )
    {
      // if there is a scan result
      if ( simpleBLEScanRes > 0 
        && simpleBLEScanIdx < simpleBLEScanRes )
      {
        // connect to current device in scan result
        peerAddr = simpleBLEDevList[simpleBLEScanIdx].addr;
        addrType = simpleBLEDevList[simpleBLEScanIdx].addrType;
      
        p->simpleBLEState = BLE_STATE_CONNECTING;
        //osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
        GAPCentralRole_EstablishLink( DEFAULT_LINK_HIGH_DUTY_CYCLE,
                                      DEFAULT_LINK_WHITE_LIST,
                                      addrType, peerAddr );
  
        LCD_WRITE_STRING( "Connecting", HAL_LCD_LINE_1 );
        LCD_WRITE_STRING( bdAddr2Str( peerAddr ), HAL_LCD_LINE_2 ); 
      }
    }
    else if ( p->simpleBLEState == BLE_STATE_CONNECTING ||
              p->simpleBLEState == BLE_STATE_CONNECTED ||
              p->simpleBLEState == BLE_STATE_DISCONNECTING)
    {
      // disconnect
      p->simpleBLEState = BLE_STATE_DISCONNECTING;

      gStatus = GAPCentralRole_TerminateLink( p->simpleBLEConnHandle );
      
      LCD_WRITE_STRING( "Disconnecting", HAL_LCD_LINE_1 ); 
    }
}
/*********************************************************************
*********************************************************************/

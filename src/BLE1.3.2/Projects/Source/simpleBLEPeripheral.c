/**************************************************************************************************
  Filename:       simpleBLEPeripheral.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

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
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"
#include "Battservice.h"

//#if defined( CC2540_MINIDK )
//  #include "simplekeys.h"
//#endif

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif
#include "Npi.h"
#include "stdio.h"
#include "osal_snv.h"
#include "simplekeys.h"
#include "dht11.h"
#include "ds18b20.h"
#include "pwm.h"

/*

AmoMcu 阿莫单片机   2014 12 08   

*/

// How often to check battery voltage (in ms)
#define BATTERY_CHECK_PERIOD                  10000


// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   1000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160       // 广播间隔， 数值越大功耗越低但是广播的包的时间间隔就太长

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

#if defined (POWER_SAVING)// 注意， 如果开启了睡眠功能， led灯就不会正常工作了， 并且连接间隔需要设较大， 才会省电
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     6//  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     6//  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据
#else
// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8//  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     8//  连接间隔与数据发送量有关， 连接间隔越短， 单位时间内就能发送越多的数据
#endif


// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         1

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000//1000  -各种原因断开连接后，超时并重新广播的时间:  100 = 1s

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         FALSE//TRUE  // 阿莫着重提醒： 这个是参数更新使能， 这里最好设置成FALSE， 否则android4.4的手机app 连接上后几秒钟后短线， 暂时用此方法解决， 应该是android 兼容问题， 后面有新的解决方法，再公告给大家， 2015 07 11 ，，amomcu 阿莫

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif

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
static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

static uint8 SensorValue[4];

// 温度传感器类型
#define SENSOR_TYPE_DHT11             0
#define SENSOR_TYPE_DS18B20           1
static uint8 g_SensorType = SENSOR_TYPE_DHT11;  // 默认传感器为dht11

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  11,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'A','m','o','S','m','a','r','t','R','F',   // 广播本地名称， 你在手机app上看到的是这个名字

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x09,   // length of this data  
  'A',
  'M',
  'O',
  'M', 
  'C',
  'U',
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),
};

// GAP GATT Attributes
//static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";   // 这个就是蓝牙设备名称
#define DEFAULT_DEVICE_NAME  "AmoSmartRF"

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void simpleProfileChangeCB( uint8 paramID );

//#if defined( CC2540_MINIDK )
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
//#endif

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
static char *bdAddr2Str ( uint8 *pAddr );
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

// 串口回调函数
static void simpleBLE_NpiSerialCallback( uint8 port, uint8 events );


/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};




typedef struct 
{
    uint8 name[GAP_DEVICE_NAME_LEN];      //本机设备名称

    uint8 pass[7];                  //密码
    
    uint8 mac_addr[B_ADDR_LEN];            //本机mac地址

/*
自动上报温度定时功能  update_time_s， 
0表示不上报， 单位 ms, 默认值  TEMP_DEFAULT_UPDATE_TIME_MS
*/
    uint32 update_time_ms;

    // 温度
    uint8 tempeature_1;    // 温度整数， 比如 12.32度的 12
    uint8 tempeature_2;    // 温度小数， 比如 12.32度的 32

    // 湿度
    uint8 humidity_1;     // 湿度整数， 比如 12.32度的 12
    uint8 humidity_2;     // 湿度小数， 比如 12.32度的 32

    // 最高温度报警
    uint16 tempeature_hight;    // 高八位是温度整数，低八位是温度小数

    // 最低温度报警
    uint16 tempeature_low;    // 高八位是温度整数，低八位是温度小数
    
    uint8 pwm[4];              // 四路 pwm 的占空比值
}SYS_CONFIG;

SYS_CONFIG sys_config;

void Init_Para(void)
{
    int8 ret8 = osal_snv_read(0x80, sizeof(SYS_CONFIG), &sys_config);
    // 如果该段内存未曾写入过数据， 直接读，会返回 NV_OPER_FAILED ,
    // 我们利用这个特点作为第一次烧录后的运行， 从而设置参数的出厂设置
    if(NV_OPER_FAILED == ret8)
    {
        // 未初始化
        osal_memset(&sys_config, 0, sizeof(SYS_CONFIG));

        sprintf((char*)sys_config.name, DEFAULT_DEVICE_NAME);  // 默认设备名称


        // 4路 pwm 设置， 相对于0xFF 的占空比
        sys_config.pwm[0] = 0x10;
        sys_config.pwm[1] = 0x20;
        sys_config.pwm[2] = 0x30;
        sys_config.pwm[3] = 0x40;
        
        osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);    // 写所有参数
    }
}

// 继电器控制   // P1.3 控制继电器的开关
void RelayONOff(bool onoff)
{
  if(onoff == TRUE)
  {
     P1SEL &= ~0x08; // 设置 io 功能
     P1DIR |= 0x08; // 输出
     P1_3 = 1;
  }
  else 
  {
     P1SEL &= ~0x08; // 设置 io 功能
     P1DIR |= 0x08; // 输出
     P1_3 = 0;
  }
}



/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;

  Init_Para();

  // 串口初始化
  NPI_InitTransport(simpleBLE_NpiSerialCallback);
  NPI_WriteTransport("SimpleBLEPeripheral_Init\r\n", 26);  
  
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    {
        //GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
        
        uint8 scanRspDataLen = 11 + osal_strlen((char*)sys_config.name);
        uint8 scanRspDataTemp[B_MAX_ADV_LEN];
        uint8 i = 0;
        scanRspDataTemp[i++] = 1 + osal_strlen((char*)sys_config.name);
        scanRspDataTemp[i++] =  GAP_ADTYPE_LOCAL_NAME_COMPLETE; // length of this data
        osal_memcpy(&scanRspDataTemp[i], sys_config.name, osal_strlen((char*)sys_config.name));
        i = 2 + osal_strlen((char*)sys_config.name);

        // connection interval range
        scanRspDataTemp[i++] = 0x05,   // length of this data
        scanRspDataTemp[i++] = GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,   
        scanRspDataTemp[i++] = LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL );   // 100ms
        scanRspDataTemp[i++] = HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL );
        scanRspDataTemp[i++] = LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL );   // 1s
        scanRspDataTemp[i++] = HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL );
        // Tx power level
        scanRspDataTemp[i++] = 0x02;   // length of this data
        scanRspDataTemp[i++] = GAP_ADTYPE_POWER_LEVEL;
        scanRspDataTemp[i++] = 0;       // 0dBm

        GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, scanRspDataLen, scanRspDataTemp );
    }


    
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, sys_config.name );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_NO_PAIRING;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  Batt_AddService( );     // Battery Service
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile

#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = {0};
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }


#if defined( CC2540_MINIDK )

  SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleBLEPeripheral_TaskID );

  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2 | HAL_LED_3), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low
#else
/*
  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low
*/
  // Register for all key events - This app will handle all key events

  HalLedSet( (HAL_LED_1 | HAL_LED_2 | HAL_LED_3), HAL_LED_MODE_OFF );

  SK_AddService( GATT_ALL_SERVICES );         // Simple Keys Profile

  RegisterForKeys( simpleBLEPeripheral_TaskID );   //  一定需要添加这个， 否则按键不起作用
#endif // #if defined( CC2540_MINIDK )

  RelayONOff(FALSE);  

  // initialize the ADC for battery reads
  HalAdcInit();

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
  #if defined (HAL_IMAGE_A)
    HalLcdWriteStringValue( "BLE Peri-A", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #else
    HalLcdWriteStringValue( "BLE Peri-B", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #endif // HAL_IMAGE_A
#else
  HalLcdWriteString( "BLE Peripheral", HAL_LCD_LINE_1 );
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &simpleBLEPeripheral_SimpleProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )

  // Setup a delayed profile startup
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );

}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL )
    {
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & SBP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );

    // Set timer for first periodic event
    osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );

    return ( events ^ SBP_START_DEVICE_EVT );
  }

  if ( events & SBP_PERIODIC_EVT )
  {

    // Perform periodic application task
    performPeriodicTask();

    osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );


    return (events ^ SBP_PERIODIC_EVT);
  }

  

  if ( events & KFD_BATTERY_CHECK_EVT )
  {
    // Restart timer
    if ( BATTERY_CHECK_PERIOD )
    {
      osal_start_timerEx( simpleBLEPeripheral_TaskID, KFD_BATTERY_CHECK_EVT, BATTERY_CHECK_PERIOD );
    }

    // perform battery level check
    Batt_MeasLevel( );

    return (events ^ KFD_BATTERY_CHECK_EVT);
  }


#if defined ( PLUS_BROADCASTER )
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  //#if defined( CC2540_MINIDK )
    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;
  //#endif // #if defined( CC2540_MINIDK )

  default:
    // do nothing
    break;
  }
}

//#if defined( CC2540_MINIDK )
/*********************************************************************
 * @fn      simpleBLEPeripheral_HandleKeys
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
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  (void)shift;  // Intentionally unreferenced parameter
  uint8 SK_Keys = keys;

  HalLcdWriteStringValue( "key = 0x", keys, 16, HAL_LCD_LINE_2 );

  // smartRF开发板上的S1 对应我们源码上的HAL_KEY_SW_6
  if ( keys & HAL_KEY_SW_6 )
  {
    HalLcdWriteString( "HAL_KEY_SW_6", HAL_LCD_LINE_3 );

    if(g_SensorType == SENSOR_TYPE_DHT11)  // 默认传感器为dht11
    {
        g_SensorType = SENSOR_TYPE_DS18B20;        
    }
    else
    {
        g_SensorType = SENSOR_TYPE_DHT11;
    }
  }

  if ( keys & HAL_KEY_UP )
  {  
    HalLcdWriteString( "HAL_KEY_UP", HAL_LCD_LINE_3 );
  }

  if ( keys & HAL_KEY_LEFT )
  {
    HalLcdWriteString( "HAL_KEY_LEFT", HAL_LCD_LINE_3 );
  }

  if ( keys & HAL_KEY_RIGHT )
  {
    HalLcdWriteString( "HAL_KEY_RIGHT", HAL_LCD_LINE_3 );
  }
  
  if ( keys & HAL_KEY_CENTER )
  {
    HalLcdWriteString( "HAL_KEY_CENTER", HAL_LCD_LINE_3 );

  }
  
  if ( keys & HAL_KEY_DOWN )
  {
    HalLcdWriteString( "HAL_KEY_DOWN", HAL_LCD_LINE_3 );
  }

  SK_SetParameter( SK_KEY_ATTR, sizeof ( uint8 ), &SK_Keys );
}
//#endif // #if defined( CC2540_MINIDK )

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          // Display device address
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_2 );
          HalLcdWriteString( "Initialized",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
          HalLcdWriteString( "", HAL_LCD_LINE_4 );
          HalLcdWriteString( "", HAL_LCD_LINE_5 );
          HalLcdWriteString( "", HAL_LCD_LINE_6 );
          HalLcdWriteString( "", HAL_LCD_LINE_7 );
          HalLcdWriteString( "", HAL_LCD_LINE_8 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

        //HalLedBlink (HAL_LED_1, 0, 10, 100);//这个的意思是， 以10%的占空比连续闪烁
          HalLedSet( (HAL_LED_1 | HAL_LED_2 | HAL_LED_3), HAL_LED_MODE_OFF );
      }
      break;

    case GAPROLE_CONNECTED:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );

          //HalLedBlink (HAL_LED_1 | HAL_LED_2 | HAL_LED_3, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
          
          HalLcdWriteString( "Key S1 = Notify CHAR6", HAL_LCD_LINE_8 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
       }
      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
    SensorValue[0] = 0;
    SensorValue[1] = 0;
    SensorValue[2] = 0;
    SensorValue[3] = 0;
     
    if(g_SensorType == SENSOR_TYPE_DHT11)  // 默认传感器为dht11
    {
        EA = 0; //  因为是 io 口读写， 所以为了避免干扰， 需要关中断
        Dht11_ReadValue(SensorValue);
        EA = 1;
        {// 显示温度， 湿度
            char str[32];
            sprintf(str, "dht11 temp: %d.%d", SensorValue[2], SensorValue[3]);
            HalLcdWriteString( str, HAL_LCD_LINE_1 );

            sprintf(str, "dht11 humi: %d.%d", SensorValue[0], SensorValue[1]);
            HalLcdWriteString( str, HAL_LCD_LINE_2 );
        }
    }
    else if(g_SensorType == SENSOR_TYPE_DS18B20)
    {
        EA = 0;//  因为是 io 口读写， 所以为了避免干扰， 需要关中断
        float ds18b20_temp = DS18B20_ReadMain();
        EA = 1;

        {// 显示温度
            char str[32];
            sprintf(str, "ds18b20 : %.2f", ds18b20_temp);
            HalLcdWriteString( "", HAL_LCD_LINE_1 );
            HalLcdWriteString( str, HAL_LCD_LINE_2 );
        }

        SensorValue[0] = 0;
        SensorValue[1] = 0;

        // 温度
        SensorValue[2] = (uint8)ds18b20_temp;
        SensorValue[3] = (uint8)( (uint16)(ds18b20_temp*100) % 100); //取小数部分
    }
}

uint8 simpleProfileReadConfig(uint16 uuid, uint8 *newValue)
{
    uint8 len = 0;

    switch ( uuid )
    {
      case SIMPLEPROFILE_CHAR1_UUID:
      case SIMPLEPROFILE_CHAR2_UUID:
      case SIMPLEPROFILE_CHAR4_UUID:
        break;

      case SIMPLEPROFILE_CHAR5_UUID:
        HalLcdWriteString( "Char 5 read", HAL_LCD_LINE_3 );
        newValue[len++] = sys_config.update_time_ms>>24;
        newValue[len++] = sys_config.update_time_ms>>16;
        newValue[len++] = sys_config.update_time_ms>>8;
        newValue[len++] = sys_config.update_time_ms>>0;
        break;

      case SIMPLEPROFILE_CHAR6_UUID:
      {
        // 注意到 SensorValue 已经在定时函数中1s更新一次了， 所以这里直接松数据即可
        
        // 湿度
        newValue[len++] = SensorValue[0];
        newValue[len++] = SensorValue[1];

        // 温度
        newValue[len++] = SensorValue[2];
        newValue[len++] = SensorValue[3];
        break;
      }
      case SIMPLEPROFILE_CHAR7_UUID:
        HalLcdWriteString( "Char 7 read", HAL_LCD_LINE_3 );
        len = osal_strlen((char*)sys_config.name);
        osal_memcpy(newValue, sys_config.name, len);        
        break;

      case SIMPLEPROFILE_CHAR8_UUID:
        HalLcdWriteString( "Char 8 read", HAL_LCD_LINE_3 );
        
        // 最高温度报警
        newValue[len++] = sys_config.tempeature_hight>>8;
        newValue[len++] = sys_config.tempeature_hight & 0xff;

        // 最低温度报警
        newValue[len++] = sys_config.tempeature_low>>8;
        newValue[len++] = sys_config.tempeature_low & 0xff;
        break;

      case SIMPLEPROFILE_CHAR9_UUID:
        HalLcdWriteString( "Char 9 read", HAL_LCD_LINE_3 );
        HalAdcSetReference( HAL_ADC_REF_AVDD );
        {
            uint16 adc4, adc5;
            // 注意， 14bit采样时，仅13bit有效，故 需要  &0x1fff, 这个就别问阿莫我了， 详见数据手册
            adc4 = HalAdcRead( HAL_ADC_CHANNEL_4, HAL_ADC_RESOLUTION_14 ) & 0x1fff;
            adc5 = HalAdcRead( HAL_ADC_CHANNEL_5, HAL_ADC_RESOLUTION_14 ) & 0x1fff;
            newValue[len++] = adc4>>8;
            newValue[len++] = adc4 & 0xFF;
            newValue[len++] = adc5>>8;
            newValue[len++] = adc5 & 0xFF;
        }
        break;

      case SIMPLEPROFILE_CHARA_UUID:// pwm
        HalLcdWriteString( "Char A read", HAL_LCD_LINE_3 );
        newValue[len++] = sys_config.pwm[0];
        newValue[len++] = sys_config.pwm[1];
        newValue[len++] = sys_config.pwm[2];
        newValue[len++] = sys_config.pwm[3];        
        break;
       
    default:
      len = 0;
    }
    
    return len;
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue[20];
  uint8 returnBytes; 

  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:  // 收到 CHAR1 的数据
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue, &returnBytes);

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
      HalLcdWriteStringValue( "Char 1 : 0x", (uint16)(newValue[0]), 16,  HAL_LCD_LINE_5 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      // 点灯控制 led1
      if(newValue[0] == 0x11)
      {
         HalLedSet( (HAL_LED_1), HAL_LED_MODE_ON );            
      }
      else if(newValue[0] == 0x10)
      {
         HalLedSet( (HAL_LED_1), HAL_LED_MODE_OFF );            
      }

      // 点灯控制 led2
      if(newValue[0] == 0x21)
      {
         HalLedSet( (HAL_LED_2), HAL_LED_MODE_ON );            
      }
      else if(newValue[0] == 0x20)
      {
         HalLedSet( (HAL_LED_2), HAL_LED_MODE_OFF );            
      }

      // 点灯控制 led3
      if(newValue[0] == 0x41)
      {
         HalLedSet( (HAL_LED_3), HAL_LED_MODE_ON );            
      }
      else if(newValue[0] == 0x40)
      {
         HalLedSet( (HAL_LED_3), HAL_LED_MODE_OFF );            
      }

      // 继电器控制   // P1.3 控制继电器的开关
      if(newValue[0] == 0x44)// 开
      {
        RelayONOff(TRUE);
      }
      else if(newValue[0] == 0x43)// 关
      {
        RelayONOff(FALSE);
      }

      //HalLedBlink (HAL_LED_1, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
     break;

    case SIMPLEPROFILE_CHAR5: // 收到 CHAR5 的数据
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR5, &newValue, &returnBytes);

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 5[0]:", (uint16)(newValue[0]), 16,  HAL_LCD_LINE_5 );
        HalLcdWriteStringValue( "Char 5[1]:", (uint16)(newValue[1]), 16,  HAL_LCD_LINE_6 );
        HalLcdWriteStringValue( "Char 5[2]:", (uint16)(newValue[2]), 16,  HAL_LCD_LINE_7 );
        HalLcdWriteStringValue( "Char 5[3]:", (uint16)(newValue[3]), 16,  HAL_LCD_LINE_8 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      // 设定温度更新时间， 注意高位在前  
      if(returnBytes == 4)
      {
        sys_config.update_time_ms = (uint32)newValue[0]<<24 | (uint32)newValue[1]<<16| (uint32)newValue[2]<<8| (uint32)newValue[3]; 
        osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);    // 写所有参数
      }


        //HalLedBlink (HAL_LED_2, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  

      break;

    case SIMPLEPROFILE_CHAR6:  // 这个是我们添加char6， 用于做串口透传 与从机 notify 数据到主机 很合适
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR6, &newValue, &returnBytes);

      {
        char str[21]={0};
        char str2[32]={0};

        osal_memcpy(str, newValue, returnBytes);
        sprintf(str2,"Char 6: %s", str);
      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteString(str2,  HAL_LCD_LINE_6 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)        

        // 通过串口透传出去，达到透传目的，amo家的透传就是使用 CHAR6
        NPI_WriteTransport(newValue, returnBytes);

        //HalLedBlink (HAL_LED_3, 1, 50, 100);//这个的意思是， 100ms内，以50%的占空比闪烁1次, 实际就是点亮50ms  
      }

      break;

    case SIMPLEPROFILE_CHAR7: // 收到 CHAR7 的数据
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR7, &newValue, &returnBytes);
      osal_memset(sys_config.name, 0, sizeof(sys_config.name));
      osal_memcpy(sys_config.name, newValue, returnBytes);
      osal_snv_write(0x80, sizeof(SYS_CONFIG), &sys_config);    // 写所有参数
      
      HalLcdWriteString( "Char 7 Set", HAL_LCD_LINE_4 );
      // 需要重启后设备名生效
      break;

    case SIMPLEPROFILE_CHAR8: // 收到 CHAR8 的数据
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR8, &newValue, &returnBytes);

      // 最高温度报警
      sys_config.tempeature_hight = newValue[0]<<8;
      sys_config.tempeature_hight |= newValue[1];
      
      // 最低温度报警
      sys_config.tempeature_low = newValue[2]<<8;
      sys_config.tempeature_low |= newValue[3];
      break;

    case SIMPLEPROFILE_CHAR9: // adc 只读
      break;

    case SIMPLEPROFILE_CHARA: // // pwm
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHARA, &newValue, &returnBytes);

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char A[0]:", (uint16)(newValue[0]), 16,  HAL_LCD_LINE_5 );
        HalLcdWriteStringValue( "Char A[1]:", (uint16)(newValue[1]), 16,  HAL_LCD_LINE_6 );
        HalLcdWriteStringValue( "Char A[2]:", (uint16)(newValue[2]), 16,  HAL_LCD_LINE_7 );
        HalLcdWriteStringValue( "Char A[3]:", (uint16)(newValue[3]), 16,  HAL_LCD_LINE_8 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      // 设定pwm
      if(returnBytes == 4)
      {
          sys_config.pwm[0] = newValue[0];
          sys_config.pwm[1] = newValue[1];
          sys_config.pwm[2] = newValue[2];
          sys_config.pwm[3] = newValue[3];
          
          // 关于pwm， 更完美的函数与设置， 请见ble高级实践19-蓝牙灯泡的源代码，自己找找了，不用问我代码在哪里。  
          PWM_Start(sys_config.pwm[0], sys_config.pwm[1], sys_config.pwm[2], sys_config.pwm[3]);
      }

      break;

    default:
      // should not reach here!
      break;
  }
}

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
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
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

// 串口回调函数
static void simpleBLE_NpiSerialCallback( uint8 port, uint8 events )
{
    (void)port;
    
    if (events & (HAL_UART_RX_TIMEOUT | HAL_UART_RX_FULL))   //串口有数据
    {
        uint8 numBytes = 0;

        numBytes = NPI_RxBufLen();           //读出串口缓冲区有多少字节
        
        if(numBytes > 0)
        {
            uint8 *buffer = osal_mem_alloc(numBytes);            
            if(buffer)
            {
                // 读出串口数据
                NPI_ReadTransport(buffer,numBytes);  
#if 1
                // 作为测试， 把读到的数据也通过串口返回， 这只是一个test功能， 你可以把去掉
                NPI_WriteTransport(buffer,numBytes);  
#endif              
                osal_mem_free(buffer);
            }
        }
    }
}

/*********************************************************************
*********************************************************************/

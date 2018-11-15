
/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "simpleGATTprofile.h"
#include "hal_lcd.h"
//#include "simpleBle.h"



/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        33//21//25


/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 simpleProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// Characteristic 1 UUID: 0xFFF1
CONST uint8 simpleProfilechar1UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR1_UUID), HI_UINT16(SIMPLEPROFILE_CHAR1_UUID)
};

// Characteristic 2 UUID: 0xFFF2
CONST uint8 simpleProfilechar2UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR2_UUID), HI_UINT16(SIMPLEPROFILE_CHAR2_UUID)
};

// Characteristic 3 UUID: 0xFFF3
CONST uint8 simpleProfilechar3UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR3_UUID), HI_UINT16(SIMPLEPROFILE_CHAR3_UUID)
};

// Characteristic 4 UUID: 0xFFF4
CONST uint8 simpleProfilechar4UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR4_UUID), HI_UINT16(SIMPLEPROFILE_CHAR4_UUID)
};

// Characteristic 5 UUID: 0xFFF5
CONST uint8 simpleProfilechar5UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR5_UUID), HI_UINT16(SIMPLEPROFILE_CHAR5_UUID)
};

// Characteristic 6 UUID: 0xFFF6
CONST uint8 simpleProfilechar6UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR6_UUID), HI_UINT16(SIMPLEPROFILE_CHAR6_UUID)
};

// Characteristic 7 UUID: 0xFFF7
CONST uint8 simpleProfilechar7UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR7_UUID), HI_UINT16(SIMPLEPROFILE_CHAR7_UUID)
};

// Characteristic 8 UUID: 0xFFF8
CONST uint8 simpleProfilechar8UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR8_UUID), HI_UINT16(SIMPLEPROFILE_CHAR8_UUID)
};

// Characteristic 9 UUID: 0xFFF9
CONST uint8 simpleProfilechar9UUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHAR9_UUID), HI_UINT16(SIMPLEPROFILE_CHAR9_UUID)
};

// Characteristic A UUID: 0xFFFA
CONST uint8 simpleProfilecharAUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARA_UUID), HI_UINT16(SIMPLEPROFILE_CHARA_UUID)
};


// Characteristic B UUID: 0xFFFB
CONST uint8 simpleProfilecharBUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARB_UUID), HI_UINT16(SIMPLEPROFILE_CHARB_UUID)
};


// Characteristic C UUID: 0xFFFC
CONST uint8 simpleProfilecharCUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARC_UUID), HI_UINT16(SIMPLEPROFILE_CHARC_UUID)
};


// Characteristic D UUID: 0xFFFD
CONST uint8 simpleProfilecharDUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARD_UUID), HI_UINT16(SIMPLEPROFILE_CHARD_UUID)
};


// Characteristic E UUID: 0xFFFE
CONST uint8 simpleProfilecharEUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARE_UUID), HI_UINT16(SIMPLEPROFILE_CHARE_UUID)
};


// Characteristic F UUID: 0xFFFF
CONST uint8 simpleProfilecharFUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_CHARF_UUID), HI_UINT16(SIMPLEPROFILE_CHARF_UUID)
};



/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static simpleProfileCBs_t *simpleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute
static CONST gattAttrType_t simpleProfileService = { ATT_BT_UUID_SIZE, simpleProfileServUUID };


// Simple Profile Characteristic 1 Properties
static uint8 simpleProfileChar1Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 1 Value
static uint8 simpleProfileChar1 = 0;

// Simple Profile Characteristic 1 User Description
static uint8 simpleProfileChar1UserDesp[17] = "Characteristic 1\0";


// Simple Profile Characteristic 2 Properties
static uint8 simpleProfileChar2Props = GATT_PROP_READ;

// Characteristic 2 Value
static uint8 simpleProfileChar2 = 0;

// Simple Profile Characteristic 2 User Description
static uint8 simpleProfileChar2UserDesp[17] = "Characteristic 2\0";


// Simple Profile Characteristic 3 Properties
static uint8 simpleProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
static uint8 simpleProfileChar3 = 0;

// Simple Profile Characteristic 3 User Description
static uint8 simpleProfileChar3UserDesp[17] = "Characteristic 3\0";


// Simple Profile Characteristic 4 Properties
static uint8 simpleProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
static uint8 simpleProfileChar4 = 0;

// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t simpleProfileChar4Config[GATT_MAX_NUM_CONN];
                                        
// Simple Profile Characteristic 4 User Description
static uint8 simpleProfileChar4UserDesp[17] = "Characteristic 4\0";


// Simple Profile Characteristic 5 Properties
static uint8 simpleProfileChar5Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 5 Value
static uint8 simpleProfileChar5[SIMPLEPROFILE_CHAR5_LEN] = {0};
static uint8 simpleProfileChar5Len = 0;

// Simple Profile Characteristic 5 User Description
static uint8 simpleProfileChar5UserDesp[26] = "Temperature Update Config\0";


// Simple Profile Characteristic 6 Properties
static uint8 simpleProfileChar6Props = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY;

// Characteristic 6 Value
static uint8 simpleProfileChar6[SIMPLEPROFILE_CHAR6_LEN] = {0};
static uint8 simpleProfileChar6Len = 0;

// Simple Profile Characteristic 6 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t simpleProfileChar6Config[GATT_MAX_NUM_CONN];

// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar6UserDesp[20] = "Current Temperature\0";


// Simple Profile Characteristic 7 Properties
static uint8 simpleProfileChar7Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 5 Value
static uint8 simpleProfileChar7[SIMPLEPROFILE_CHAR7_LEN] = {0};
static uint8 simpleProfileChar7Len = 0;

// Simple Profile Characteristic 7 User Description
static uint8 simpleProfileChar7UserDesp[19] = "Device Name Config\0";


// Simple Profile Characteristic 8 Properties
static uint8 simpleProfileChar8Props = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 8 Value
static uint8 simpleProfileChar8[SIMPLEPROFILE_CHAR8_LEN] = {0};
static uint8 simpleProfileChar8Len = 0;

// Simple Profile Characteristic 8 User Description
static uint8 simpleProfileChar8UserDesp[18] = "Temperature Alert\0";



// Simple Profile Characteristic 9 Properties
static uint8 simpleProfileChar9Props = GATT_PROP_READ;

// Characteristic 9 Value
static uint8 simpleProfileChar9[SIMPLEPROFILE_CHAR9_LEN] = {0};
static uint8 simpleProfileChar9Len = 0;

// Simple Profile Characteristic 9 User Description
static uint8 simpleProfileChar9UserDesp[27] = "ADC4 ADC5 Read Value 14Bit\0";



// Simple Profile Characteristic A Properties
static uint8 simpleProfileCharAProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic A Value
static uint8 simpleProfileCharA[SIMPLEPROFILE_CHARA_LEN] = {0};
static uint8 simpleProfileCharALen = 0;

// Simple Profile Characteristic A User Description
static uint8 simpleProfileCharAUserDesp[11] = "PWM Config\0";

// Simple Profile Characteristic B Properties
static uint8 simpleProfileCharBProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic B Value
static uint8 simpleProfileCharB[SIMPLEPROFILE_CHARB_LEN] = {0};
static uint8 simpleProfileCharBLen = 0;

// Simple Profile Characteristic B User Description
static uint8 simpleProfileCharBUserDesp[11] = "PAS Config\0";



// Simple Profile Characteristic C Properties
static uint8 simpleProfileCharCProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic C Value
static uint8 simpleProfileCharC[SIMPLEPROFILE_CHARC_LEN] = {0};
static uint8 simpleProfileCharCLen = 0;

// Simple Profile Characteristic C User Description
static uint8 simpleProfileCharCUserDesp[11] = "PAS Config\0";

// Simple Profile Characteristic D Properties
static uint8 simpleProfileCharDProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic D Value
static uint8 simpleProfileCharD[SIMPLEPROFILE_CHARD_LEN] = {0};
static uint8 simpleProfileCharDLen = 0;

// Simple Profile Characteristic D User Description
static uint8 simpleProfileCharDUserDesp[11] = "PAS Config\0";

// Simple Profile Characteristic E Properties
static uint8 simpleProfileCharEProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic E Value
static uint8 simpleProfileCharE[SIMPLEPROFILE_CHARE_LEN] = {0};
static uint8 simpleProfileCharELen = 0;

// Simple Profile Characteristic E User Description
static uint8 simpleProfileCharEUserDesp[11] = "PAS Config\0";

// Simple Profile Characteristic F Properties
static uint8 simpleProfileCharFProps = GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic F Value
static uint8 simpleProfileCharF[SIMPLEPROFILE_CHARF_LEN] = {0};
static uint8 simpleProfileCharFLen = 0;

// Simple Profile Characteristic F User Description
static uint8 simpleProfileCharFUserDesp[11] = "PAS Config\0";


/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t simpleProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Simple Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&simpleProfileService            /* pValue */
  },

    // Characteristic 1 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar1Props 
    },

      // Characteristic Value 1
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar1UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        &simpleProfileChar1 
      },

      // Characteristic 1 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar1UserDesp 
      },      

    // Characteristic 2 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar2Props 
    },

      // Characteristic Value 2
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar2UUID },
        GATT_PERMIT_READ, 
        0, 
        &simpleProfileChar2 
      },

      // Characteristic 2 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar2UserDesp 
      },           
      
    // Characteristic 3 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar3Props 
    },

      // Characteristic Value 3
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar3UUID },
        GATT_PERMIT_WRITE, 
        0, 
        &simpleProfileChar3 
      },

      // Characteristic 3 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar3UserDesp 
      },

    // Characteristic 4 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar4Props 
    },

      // Characteristic Value 4
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar4UUID },
        0, 
        0, 
        &simpleProfileChar4 
      },

      // Characteristic 4 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)simpleProfileChar4Config 
      },
      
      // Characteristic 4 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar4UserDesp 
      },
      
    // Characteristic 5 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar5Props 
    },

      // Characteristic Value 5
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar5UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileChar5 
      },

      // Characteristic 5 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar5UserDesp 
      },
      
    // Characteristic 6 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar6Props 
    },

      // Characteristic Value 6
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar6UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileChar6 
      },

      // Characteristic 6 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)simpleProfileChar6Config 
      },

      // Characteristic 6 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar6UserDesp 
      },
#if 1
    // Characteristic 7 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar7Props 
    },

      // Characteristic Value 7
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar7UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileChar7 
      },

      // Characteristic 7 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar7UserDesp 
      },
#endif    

#if 1
    // Characteristic 8 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar8Props 
    },

      // Characteristic Value 8
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar8UUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileChar8
      },

      // Characteristic 8 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar8UserDesp 
      },
#endif      

#if 1
    // Characteristic 9 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar9Props 
    },

      // Characteristic Value 9
      { 
        { ATT_BT_UUID_SIZE, simpleProfilechar9UUID },
        GATT_PERMIT_READ,
        0, 
        simpleProfileChar9
      },

      // Characteristic 9 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar9UserDesp 
      },
#endif      

#if 1
    // Characteristic A Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharAProps 
    },

      // Characteristic Value A
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharAUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharA 
      },

      // Characteristic A User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharAUserDesp 
      },
#endif   

#if 0
    // Characteristic B Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharBProps 
    },

      // Characteristic Value B
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharBUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharB 
      },

      // Characteristic B User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharBUserDesp 
      },
#endif  

#if 0
    // Characteristic C Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharCProps 
    },

      // Characteristic Value C
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharCUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharC 
      },

      // Characteristic C User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharCUserDesp 
      },
#endif  

#if 0
    // Characteristic D Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharDProps 
    },

      // Characteristic Value D
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharDUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharD 
      },

      // Characteristic D User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharDUserDesp 
      },
#endif  


#if 0
    // Characteristic E Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharEProps 
    },

      // Characteristic Value E
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharEUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharE 
      },

      // Characteristic E User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharEUserDesp 
      },
#endif  

#if 0
    // Characteristic F Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileCharFProps 
    },

      // Characteristic Value F
      { 
        { ATT_BT_UUID_SIZE, simpleProfilecharFUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        simpleProfileCharF 
      },

      // Characteristic F User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileCharFUserDesp 
      },
#endif  


};


/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8 simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );

static void simpleProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType );


/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t simpleProfileCBs =
{
  simpleProfile_ReadAttrCB,  // Read callback function pointer
  simpleProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t SimpleProfile_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar4Config );
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar6Config );
  //GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar7Config );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( simpleProfile_HandleConnStatusCB );  
  
  if ( services & SIMPLEPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( simpleProfileAttrTbl, 
                                          GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                          &simpleProfileCBs );
  }

  return ( status );
}


/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs( simpleProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    simpleProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}
  

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to right
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case SIMPLEPROFILE_CHAR1:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar1 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SIMPLEPROFILE_CHAR2:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar2 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SIMPLEPROFILE_CHAR3:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar3 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SIMPLEPROFILE_CHAR4:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar4 = *((uint8*)value);
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( simpleProfileChar4Config, &simpleProfileChar4, FALSE,
                                    simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                    INVALID_TASK_ID );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case SIMPLEPROFILE_CHAR5:
      ret = bleInvalidRange;
      break;
      
    case SIMPLEPROFILE_CHAR6:
      //LCD_WRITE_STRING_VALUE( "SetParameter 6 len=", len, 10, HAL_LCD_LINE_1 );
      //if ( len == SIMPLEPROFILE_CHAR6_LEN ) 
      if ( len <= SIMPLEPROFILE_CHAR6_LEN ) 
      {
        VOID osal_memcpy( simpleProfileChar6, value, len );
        simpleProfileChar6Len = len;
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( simpleProfileChar6Config, simpleProfileChar6, FALSE,
                                    simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                    INVALID_TASK_ID );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    case SIMPLEPROFILE_CHAR7:
    case SIMPLEPROFILE_CHAR8:
    case SIMPLEPROFILE_CHAR9:
    case SIMPLEPROFILE_CHARA:
      ret = bleInvalidRange;
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_GetParameter( uint8 param, void *value, uint8 *returnBytes)
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case SIMPLEPROFILE_CHAR1:
      *((uint8*)value) = simpleProfileChar1;
      *returnBytes = 1;
      break;

    case SIMPLEPROFILE_CHAR2:
      *((uint8*)value) = simpleProfileChar2;
      *returnBytes = 1;
      break;      

    case SIMPLEPROFILE_CHAR3:
      *((uint8*)value) = simpleProfileChar3;
      *returnBytes = 1;
      break;  

    case SIMPLEPROFILE_CHAR4:
      *((uint8*)value) = simpleProfileChar4;
      *returnBytes = 1;
      break;

    case SIMPLEPROFILE_CHAR5:
      VOID osal_memcpy( value, simpleProfileChar5, simpleProfileChar5Len );
      *returnBytes = simpleProfileChar5Len;
      break;      
      
    case SIMPLEPROFILE_CHAR6:
      VOID osal_memcpy( value, simpleProfileChar6, simpleProfileChar6Len );
      *returnBytes = simpleProfileChar6Len;
      break;  
      
    case SIMPLEPROFILE_CHAR7:
      VOID osal_memcpy( value, simpleProfileChar7, simpleProfileChar7Len );
      *returnBytes = simpleProfileChar7Len;
      break;  
      
    case SIMPLEPROFILE_CHAR8:
      VOID osal_memcpy( value, simpleProfileChar8, simpleProfileChar8Len );
      *returnBytes = simpleProfileChar8Len;
      break;  

    case SIMPLEPROFILE_CHAR9:
      VOID osal_memcpy( value, simpleProfileChar9, simpleProfileChar9Len );
      *returnBytes = simpleProfileChar9Len;
      break;  

    case SIMPLEPROFILE_CHARA:
      VOID osal_memcpy( value, simpleProfileCharA, simpleProfileCharALen );
      *returnBytes = simpleProfileCharALen;
      break;  

    default:
      ret = INVALIDPARAMETER;
      *returnBytes = 0;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static uint8 simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads

      // characteristics 1 and 2 have read permissions
      // characteritisc 3 does not have read permissions; therefore it is not
      //   included here
      // characteristic 4 does not have read permissions, but because it
      //   can be sent as a notification, it is included here
      case SIMPLEPROFILE_CHAR1_UUID:
      case SIMPLEPROFILE_CHAR2_UUID:
      case SIMPLEPROFILE_CHAR4_UUID:
        *pLen = 1;
        pValue[0] = *pAttr->pValue;
        break;

      case SIMPLEPROFILE_CHAR6_UUID:
#if 0
        //*Len = SIMPLEPROFILE_CHAR6_LEN;
        //VOID osal_memcpy( pValue, pAttr->pValue, SIMPLEPROFILE_CHAR6_LEN );
        //LCD_WRITE_STRING_VALUE( "ReadAttrCB 6 len=", simpleProfileChar6Len, 10, HAL_LCD_LINE_1 );
        *pLen = simpleProfileChar6Len;
        VOID osal_memcpy( pValue, pAttr->pValue, simpleProfileChar6Len );
        {
          // 这个变量用于表明上一次写数据到从机已经成功， 可用于判断写数据时的判断， 以确保数据的完整性
          //extern bool simpleBLEChar6DoWrite2;
          //simpleBLEChar6DoWrite2 = TRUE;
        }
        break;
#endif
      case SIMPLEPROFILE_CHAR5_UUID:
      case SIMPLEPROFILE_CHAR7_UUID:
      case SIMPLEPROFILE_CHAR8_UUID:
      case SIMPLEPROFILE_CHAR9_UUID:
      case SIMPLEPROFILE_CHARA_UUID:
        {
            uint8 newValue[20];
            extern uint8 simpleProfileReadConfig(uint16 uuid, uint8 *newValue);            
            *pLen = simpleProfileReadConfig(uuid, newValue);
            VOID osal_memcpy( pValue, newValue, *pLen );
        }
        break;
        
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */
static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      case SIMPLEPROFILE_CHAR1_UUID:
      case SIMPLEPROFILE_CHAR3_UUID:

        //Validate the value
        // Make sure it's not a blob oper
        if ( offset == 0 )
        {
          if ( len != 1 )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }
        
        //Write the value
        if ( status == SUCCESS )
        {
          uint8 *pCurValue = (uint8 *)pAttr->pValue;        
          *pCurValue = pValue[0];

          if( pAttr->pValue == &simpleProfileChar1 )
          {
            notifyApp = SIMPLEPROFILE_CHAR1;        
          }
          else
          {
            notifyApp = SIMPLEPROFILE_CHAR3;           
          }
        }
             
        break;

      case SIMPLEPROFILE_CHAR5_UUID:
        if ( offset == 0 )
        {
          //if ( len != SIMPLEPROFILE_CHAR6_LEN )
          if ( len > SIMPLEPROFILE_CHAR5_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileChar5Len = len;
            notifyApp = SIMPLEPROFILE_CHAR5;
        }
             
        break;        
        
      case SIMPLEPROFILE_CHAR6_UUID:
        //Validate the value
        // Make sure it's not a blob oper
        //LCD_WRITE_STRING_VALUE( "WriteAttrCB 6 len=", len, 10, HAL_LCD_LINE_1 );
        //LCD_WRITE_STRING_VALUE( "WriteAttrCB 6 len2=", simpleProfileChar6Len, 10, HAL_LCD_LINE_1 );
        if ( offset == 0 )
        {
          //if ( len != SIMPLEPROFILE_CHAR6_LEN )
          if ( len > SIMPLEPROFILE_CHAR6_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        //VOID osal_memcpy( pAttr->pValue, pValue, SIMPLEPROFILE_CHAR6_LEN );
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileChar6Len = len;
            notifyApp = SIMPLEPROFILE_CHAR6;
        }
             
        break;

      case SIMPLEPROFILE_CHAR7_UUID:
        if ( offset == 0 )
        {
          if ( len > SIMPLEPROFILE_CHAR7_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileChar7Len = len;
            notifyApp = SIMPLEPROFILE_CHAR7;
        }
             
        break;        

      case SIMPLEPROFILE_CHAR8_UUID:
        if ( offset == 0 )
        {
          if ( len > SIMPLEPROFILE_CHAR8_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileChar8Len = len;
            notifyApp = SIMPLEPROFILE_CHAR8;
        }
             
        break;        

      case SIMPLEPROFILE_CHAR9_UUID:
        if ( offset == 0 )
        {
          if ( len > SIMPLEPROFILE_CHAR9_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileChar9Len = len;
            notifyApp = SIMPLEPROFILE_CHAR9;
        }
             
        break;       

      case SIMPLEPROFILE_CHARA_UUID:
        if ( offset == 0 )
        {
          if ( len > SIMPLEPROFILE_CHARA_LEN )
          {
            status = ATT_ERR_INVALID_VALUE_SIZE;
          }
        }
        else
        {
          status = ATT_ERR_ATTR_NOT_LONG;
        }

        //Write the value
        if ( status == SUCCESS )
        {
	        VOID osal_memcpy( pAttr->pValue, pValue, len );
            simpleProfileCharALen = len;
            notifyApp = SIMPLEPROFILE_CHARA;
        }
             
        break;        

      case GATT_CLIENT_CHAR_CFG_UUID:
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
        break;
        
      default:
        // Should never get here! (characteristics 2 and 4 do not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange )
  {
    simpleProfile_AppCBs->pfnSimpleProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void simpleProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
        GATTServApp_InitCharCfg( connHandle/*INVALID_CONNHANDLE*/, simpleProfileChar4Config );
        GATTServApp_InitCharCfg( connHandle/*INVALID_CONNHANDLE*/, simpleProfileChar6Config );
    }
  }
}


/*********************************************************************
*********************************************************************/

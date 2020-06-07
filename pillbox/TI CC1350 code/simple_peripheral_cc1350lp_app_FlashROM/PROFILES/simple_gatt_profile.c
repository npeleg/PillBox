/******************************************************************************

 @file  simple_gatt_profile.c

 @brief This file contains the Simple GATT profile sample GATT service profile
        for use with the BLE sample application.

 Group: WCS, BTS
 $Target Device: DEVICES $

 ******************************************************************************
 $License: BSD3 2010 $
 ******************************************************************************
 $Release Name: PACKAGE NAME $
 $Release Date: PACKAGE RELEASE DATE $
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <xdc/runtime/System.h>

/* for non-volatile flash memory use */
#include <ti/drivers/NVS.h>

#include <Board.h>

#include "bcomdef.h"
#include "osal.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "simple_gatt_profile.h"
#include "./Application/profile_variables.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED       17

#define Board_NVSINTERNAL       CC1350_LAUNCHXL_NVSCC26XX0

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 simpleProfileServUUID[ATT_BT_UUID_SIZE] =
{ 
  LO_UINT16(SIMPLEPROFILE_SERV_UUID), HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// currentHour
CONST uint8 simpleProfilechar6UUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SIMPLEPROFILE_CHAR6_UUID), HI_UINT16(SIMPLEPROFILE_CHAR6_UUID)
};

// currentMinutes
CONST uint8 simpleProfilechar7UUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SIMPLEPROFILE_CHAR7_UUID), HI_UINT16(SIMPLEPROFILE_CHAR7_UUID)
};

// morningTime
CONST uint8 simpleProfilechar8UUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SIMPLEPROFILE_CHAR8_UUID), HI_UINT16(SIMPLEPROFILE_CHAR8_UUID)
};

// noonTime
CONST uint8 simpleProfilechar9UUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SIMPLEPROFILE_CHAR9_UUID), HI_UINT16(SIMPLEPROFILE_CHAR9_UUID)
};
 //eveningTime
CONST uint8 simpleProfilechar10UUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16(SIMPLEPROFILE_CHAR10_UUID), HI_UINT16(SIMPLEPROFILE_CHAR10_UUID)
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


// Simple Profile Characteristic 6 Properties
static uint8 simpleProfileChar6Props =  GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 6 Value
uint8 currentHour = 0;
//static uint8 simpleChar6Len = 0;

static gattCharCfg_t *simpleProfileChar6Config;


// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar6UserDesp[13] = "Current Hour";

static uint8 simpleProfileChar7Props =  GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 6 Value
uint8 currentMinutes = 0;
//static uint8 simpleChar7Len = 0;/

// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar7UserDesp[17] = "Current Minutes";


static uint8 simpleProfileChar8Props =  GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 6 Value
//static uint8 morningTime = 0;
uint8 morningTime = 0;

// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar8UserDesp[13] = "Morning Time";


static uint8 simpleProfileChar9Props =  GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 6 Value
uint8 noonTime = 0;

// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar9UserDesp[15] = "Afternoon Time";


static uint8 simpleProfileChar10Props =  GATT_PROP_READ | GATT_PROP_WRITE;

// Characteristic 6 Value
uint8 eveningTime = 0;

// Simple Profile Characteristic 6 User Description
static uint8 simpleProfileChar10UserDesp[13] = "Evening Time";





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
             &currentHour
           },

           {
             { ATT_BT_UUID_SIZE, clientCharCfgUUID },
             GATT_PERMIT_READ | GATT_PERMIT_WRITE,
             0,
             (uint8 *)&simpleProfileChar6Config
           },

           // Characteristic 6 User Description
           {
             { ATT_BT_UUID_SIZE, charUserDescUUID },
             GATT_PERMIT_READ,
             0,
             simpleProfileChar6UserDesp
           },
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
                  &currentMinutes
                },

                // Characteristic 7 User Description
                {
                  { ATT_BT_UUID_SIZE, charUserDescUUID },
                  GATT_PERMIT_READ,
                  0,
                  simpleProfileChar7UserDesp
                },


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
                      &morningTime
                    },

                    // Characteristic 8 User Description
                    {
                      { ATT_BT_UUID_SIZE, charUserDescUUID },
                      GATT_PERMIT_READ,
                      0,
                      simpleProfileChar8UserDesp
                    },

                    // Characteristic 9 Declaration
                      {
                        { ATT_BT_UUID_SIZE, characterUUID },
                        GATT_PERMIT_READ,
                        0,
                        &simpleProfileChar9Props
                      },

                        // Characteristic Value 8
                        {
                          { ATT_BT_UUID_SIZE, simpleProfilechar9UUID },
                          GATT_PERMIT_READ | GATT_PERMIT_WRITE,
                          0,
                          &noonTime
                        },

                        // Characteristic 8 User Description
                        {
                          { ATT_BT_UUID_SIZE, charUserDescUUID },
                          GATT_PERMIT_READ,
                          0,
                          simpleProfileChar9UserDesp
                        },


                        // Characteristic 10 Declaration
                          {
                            { ATT_BT_UUID_SIZE, characterUUID },
                            GATT_PERMIT_READ,
                            0,
                            &simpleProfileChar10Props
                          },

                            // Characteristic Value 8
                            {
                              { ATT_BT_UUID_SIZE, simpleProfilechar10UUID },
                              GATT_PERMIT_READ | GATT_PERMIT_WRITE,
                              0,
                              &eveningTime
                            },

                            // Characteristic 8 User Description
                            {
                              { ATT_BT_UUID_SIZE, charUserDescUUID },
                              GATT_PERMIT_READ,
                              0,
                              simpleProfileChar10UserDesp
                            },


};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr, 
                                          uint8 *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Simple Profile Service Callbacks
// Note: When an operation on a characteristic requires authorization and 
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the 
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an 
// operation on a characteristic requires authorization the Stack will call 
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be 
// made within these functions.
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
  uint8 status;

  // Allocate Client Characteristic Configuration table
  simpleProfileChar6Config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
  if ( simpleProfileChar6Config == NULL )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar6Config );
  
  if ( services & SIMPLEPROFILE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( simpleProfileAttrTbl, 
                                          GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &simpleProfileCBs );
  }
  else
  {
    status = SUCCESS;
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


void SetParameter2( uint8 param, uint8 len, uint8 *value ) {

    //int i;
    //uint8 * character;
    //uint8 * char_len;

    switch (param)
    {
    case SIMPLEPROFILE_CHAR6:
        //character = currentHour;
        //char_len = &simpleChar6Len;
        currentHour = *(value);
        return;

    case SIMPLEPROFILE_CHAR7:
        //character = currentMinutes;
        //char_len = &simpleChar7Len;
        currentMinutes = *(value);
        return;

    case SIMPLEPROFILE_CHAR8:
        morningTime = *(value);
        //System_printf("char 8 is now: %u\n", morningTime);
        return;

    case SIMPLEPROFILE_CHAR9:
        noonTime = *(value);
        //System_printf("char 8 is now: %u\n", morningTime);
        return;

    case SIMPLEPROFILE_CHAR10:
        eveningTime = *(value);
        //System_printf("char 8 is now: %u\n", morningTime);
        return;

    default:
        break;
        // Should not get here!
    }

}



/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
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
    case SIMPLEPROFILE_CHAR6:
        currentHour = *((uint8*)value);
      break;

    case SIMPLEPROFILE_CHAR7:
        currentMinutes = *((uint8*)value);
      break;

    case SIMPLEPROFILE_CHAR8:
        morningTime = *((uint8*)value);

    case SIMPLEPROFILE_CHAR9:
        noonTime = *((uint8*)value);

    case SIMPLEPROFILE_CHAR10:
        eveningTime = *((uint8*)value);

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
bStatus_t SimpleProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case SIMPLEPROFILE_CHAR6:
      *((uint8*)value) = currentHour;
      break;

    case SIMPLEPROFILE_CHAR7:
      *((uint8*)value) = currentMinutes;
      break;

    case SIMPLEPROFILE_CHAR8:
      *((uint8*)value) = morningTime;
      break;

    case SIMPLEPROFILE_CHAR9:
      *((uint8*)value) = noonTime;
      break;

    case SIMPLEPROFILE_CHAR10:
      *((uint8*)value) = eveningTime;
      break;

    default:
      ret = INVALIDPARAMETER;
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
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8 *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method)
{
  bStatus_t status = SUCCESS;
  
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
      case SIMPLEPROFILE_CHAR6_UUID:

        if(currentHour >= 10) {
            *pLen = 2;
        }
        else {
            *pLen = 1;
        }
        *pValue = currentHour;
        break;

      case SIMPLEPROFILE_CHAR7_UUID:
          if(currentMinutes >= 10) {
              *pLen = 2;
          }
          else {
              *pLen = 1;
          }
          *pValue = currentMinutes;
        break;

      case SIMPLEPROFILE_CHAR8_UUID:
          if(morningTime >= 10) {
              *pLen = 2;
          }
          else {
              *pLen = 1;
          }
          *pValue = morningTime;
        break;

      case SIMPLEPROFILE_CHAR9_UUID:
          if(noonTime >= 10) {
              *pLen = 2;
          }
          else {
              *pLen = 1;
          }
          *pValue = noonTime;
        break;

      case SIMPLEPROFILE_CHAR10_UUID:
          if(eveningTime >= 10) {
              *pLen = 2;
          }
          else {
              *pLen = 1;
          }
          *pValue = eveningTime;
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
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    uint8 finalValue = 1;
    uint8 *pCurValue;
    switch ( uuid )
    {
      case SIMPLEPROFILE_CHAR6_UUID:
        if(len == 2) {
            finalValue = 1;
            finalValue = finalValue * (pValue[0] - '0') * 10;
            finalValue = finalValue + (pValue[1] - '0');;
        }
        else {
            finalValue = finalValue * (pValue[0] - '0');
        }
        pCurValue = (uint8 *)&currentHour;
        *pCurValue = finalValue;
        return SUCCESS;

      case SIMPLEPROFILE_CHAR7_UUID:
          if(len == 2) {
              finalValue = 1;
              finalValue = finalValue * (pValue[0] - '0') * 10;
              finalValue = finalValue + (pValue[1] - '0');;
          }
          else {
              finalValue = *pValue;
          }
          pCurValue = (uint8 *)&currentMinutes;
          *pCurValue = finalValue;
        return SUCCESS;

      case SIMPLEPROFILE_CHAR8_UUID:
          if(len == 2) {
              finalValue = 1;
              finalValue = finalValue * (pValue[0] - '0') * 10;
              finalValue = finalValue + (pValue[1] - '0');;
          }
          else {
              finalValue = *pValue;
          }
          pCurValue = (uint8 *)&morningTime;
          *pCurValue = finalValue;
        return SUCCESS;

      case SIMPLEPROFILE_CHAR9_UUID:
          if(len == 2) {
              finalValue = 1;
              finalValue = finalValue * (pValue[0] - '0') * 10;
              finalValue = finalValue + (pValue[1] - '0');;
          }
          else {
              finalValue = *pValue;
          }
          pCurValue = (uint8 *)&noonTime;
          *pCurValue = finalValue;
        return SUCCESS;

      case SIMPLEPROFILE_CHAR10_UUID:
          if(len == 2) {
              finalValue = 1;
              finalValue = finalValue * (pValue[0] - '0') * 10;
              finalValue = finalValue + (pValue[1] - '0');;
          }
          else {
              finalValue = *pValue;
          }
          pCurValue = (uint8 *)&eveningTime;
          *pCurValue = finalValue;
        return SUCCESS;

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

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnSimpleProfileChange )
  {
    simpleProfile_AppCBs->pfnSimpleProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/

/******************************************************************************

 @file  oad.h

 @brief This file contains OAD Profile header file.

 Group: WCS, BTS
 $Target Device: DEVICES $

 ******************************************************************************
 $License: BSD3 2012 $
 ******************************************************************************
 $Release Name: PACKAGE NAME $
 $Release Date: PACKAGE RELEASE DATE $
 *****************************************************************************/
#ifndef OAD_H
#define OAD_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
#define OAD_SERVICE_UUID       0xFFC0
#define OAD_IMG_IDENTIFY_UUID  0xFFC1
#define OAD_IMG_BLOCK_UUID     0xFFC2
#define OAD_IMG_COUNT_UUID     0xFFC3
#define OAD_IMG_STATUS_UUID    0xFFC4

#define OAD_RESET_SERVICE_UUID 0xFFD0
#define OAD_RESET_CHAR_UUID    0xFFD1

// OAD Characteristic Indices
#define OAD_IDX_IMG_IDENTIFY   0
#define OAD_IDX_IMG_BLOCK      1
#define OAD_IDX_IMG_COUNT      2
#define OAD_IDX_IMG_STATUS     3

// Number of characteristics in the service
#define OAD_CHAR_CNT           4

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*oadWriteCB_t)(uint8_t event, uint16_t connHandle,
                             uint8_t *pData);

typedef struct
{
  oadWriteCB_t       pfnOadWrite; // Called when characteristic value changes.
} oadTargetCBs_t;

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OAD_addService
 *
 * @brief   Initializes the OAD Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @param   None.
 *
 * @return  Success or Failure
 */
extern uint8_t OAD_addService(void);

#ifdef IMAGE_INVALIDATE
/*********************************************************************
 * @fn      Reset_addService
 *
 * @brief   Initializes the Reset Service by registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @param   None.
 *
 * @return  Success or Failure
 */
extern uint8_t Reset_addService(void);
#endif // IMAGE_INVALIDATE

/*********************************************************************
 * @fn      OAD_register
 *
 * @brief   Register a callback function with the OAD Target Profile.
 *
 * @param   pfnOadCBs - struct holding function pointers to OAD application
 *                      callbacks.
 *
 * @return  None.
 */
extern void OAD_register(oadTargetCBs_t *pfnOadCBs);

/*********************************************************************
 * @fn      OAD_imgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.  Determine from the received OAD
 *          Image Header if the Downloaded Image should be acquired.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue     - pointer to data to be written
 *
 * @return  None.
 */
extern void OAD_imgIdentifyWrite(uint16 connHandle, uint8 *pValue);

/*********************************************************************
 * @fn      OAD_imgBlockWrite
 *
 * @brief   Process the Image Block Write.
 *
 * @param   connHandle - connection message was received on
 * @param   pValue     - pointer to data to be written
 *
 * @return  None.
 */
extern void OAD_imgBlockWrite(uint16 connHandle, uint8 *pValue);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OAD_H */

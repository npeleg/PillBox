/******************************************************************************

 @file  main.c

 @brief main entry of the BLE stack sample application.

 Group: WCS, BTS
 Target Device: cc13x0

 ******************************************************************************
 
 Copyright (c) 2013-2019, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/
  
/*******************************************************************************
 * INCLUDES
 */

//#include <unistd.h>
//
///* for Time Management */
//#include <time.h>
//#include <ti/sysbios/hal/Seconds.h>
//
///* for System_printf */
//#include <xdc/runtime/System.h>
//
#include <stdio.h>
#include <stdlib.h>

/* Driver Header files */
#include <ti/drivers/PIN.h>
#include <ti/drivers/ADC.h>

//
///* Example/Board Header files */
#include "Board.h"
//#include <unistd.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Queue.h>

/* For stdout prints */
#include <xdc/runtime/System.h>

#include <xdc/runtime/Error.h>
#include <xdc/std.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/sysbios/BIOS.h>

#include "icall.h"
#include "hal_assert.h"
#include "bcomdef.h"
#include "peripheral.h"
#include "simple_peripheral.h"

#include "./Application/profile_variables.h"

/* Header files required to enable instruction fetch cache */
#include <inc/hw_memmap.h>
#include <driverlib/vims.h>

#ifndef USE_DEFAULT_USER_CFG

#include "ble_user_config.h"

void Sadna_createTask(void);
// BLE user defined configuration
bleUserCfg_t user0Cfg = BLE_USER_CFG;

#endif // USE_DEFAULT_USER_CFG

#ifdef USE_CORE_SDK
  #include <ti/display/Display.h>
#else // !USE_CORE_SDK
  #include <ti/mw/display/Display.h>
#endif // USE_CORE_SDK

#ifdef USE_FPGA
#include <inc/hw_prcm.h>
#endif // USE_FPGA

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

#if defined( USE_FPGA )
  #define RFC_MODE_BLE                 PRCM_RFCMODESEL_CURR_MODE1
  #define RFC_MODE_ANT                 PRCM_RFCMODESEL_CURR_MODE4
  #define RFC_MODE_EVERYTHING_BUT_ANT  PRCM_RFCMODESEL_CURR_MODE5
  #define RFC_MODE_EVERYTHING          PRCM_RFCMODESEL_CURR_MODE6
  //
  #define SET_RFC_BLE_MODE(mode) HWREG( PRCM_BASE + PRCM_O_RFCMODESEL ) = (mode)
#endif // USE_FPGA

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

#ifdef CC1350_LAUNCHXL
#ifdef POWER_SAVING
// Power Notify Object for wake-up callbacks
Power_NotifyObj rFSwitchPowerNotifyObj;
static uint8_t rFSwitchNotifyCb(uint8_t eventType, uint32_t *eventArg,
                                uint32_t *clientArg);
#endif //POWER_SAVING

PIN_State  radCtrlState;
PIN_Config radCtrlCfg[] = 
{
  Board_DIO1_RFSW   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW  | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* RF SW Switch defaults to 2.4GHz path*/
  Board_DIO30_SWPWR | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* Power to the RF Switch */
  PIN_TERMINATE
};
PIN_Handle radCtrlHandle;
#endif //CC1350_LAUNCHXL

#define LIMIT                   3200122890
#define TICKS_PER_SECOND        32768
#define SNOOZETIME              1800

//PIN_Config LedPinTable[] =
//{
//    Board_PIN_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially off */
//    Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially off */
//    PIN_TERMINATE                                                                      /* Terminate list */
//};

/*******************************************************************************
 * EXTERNS
 */

extern void AssertHandler(uint8 assertCause, uint8 assertSubcause);

extern Display_Handle dispHandle;

/*******************************************************************************
 * @fn          Main
 *
 * @brief       Application Main
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
int main()
{
#if defined( USE_FPGA )
  HWREG(PRCM_BASE + PRCM_O_PDCTL0) &= ~PRCM_PDCTL0_RFC_ON;
  HWREG(PRCM_BASE + PRCM_O_PDCTL1) &= ~PRCM_PDCTL1_RFC_ON;
#endif // USE_FPGA
  
  /* Register Application callback to trap asserts raised in the Stack */
  RegisterAssertCback(AssertHandler);

  PIN_init(BoardGpioInitTable);

#ifdef CC1350_LAUNCHXL
  // Enable 2.4GHz Radio
  radCtrlHandle = PIN_open(&radCtrlState, radCtrlCfg);
  
#ifdef POWER_SAVING
  Power_registerNotify(&rFSwitchPowerNotifyObj, 
                       PowerCC26XX_ENTERING_STANDBY | PowerCC26XX_AWAKE_STANDBY,
                       (Power_NotifyFxn) rFSwitchNotifyCb, NULL);
#endif //POWER_SAVING
#endif //CC1350_LAUNCHXL

#if defined( USE_FPGA )
  // set RFC mode to support BLE
  // Note: This must be done before the RF Core is released from reset!
  SET_RFC_BLE_MODE(RFC_MODE_BLE);
#endif // USE_FPGA
  
  // Enable iCache prefetching
  VIMSConfigure(VIMS_BASE, TRUE, TRUE);

  // Enable cache
  VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);

#if !defined( POWER_SAVING ) || defined( USE_FPGA )
  /* Set constraints for Standby, powerdown and idle mode */
  // PowerCC26XX_SB_DISALLOW may be redundant
  Power_setConstraint(PowerCC26XX_SB_DISALLOW);
  Power_setConstraint(PowerCC26XX_IDLE_PD_DISALLOW);
#endif // POWER_SAVING | USE_FPGA

  /* Initialize ICall module */
  ICall_init();

  System_printf("icall init");
  System_flush();

  /* Start tasks of external images - Priority 5 */
  ICall_createRemoteTasks();

  System_printf("icall remote tasks");
  System_flush();

  /* Kick off profile - Priority 3 */
  GAPRole_createTask();

  System_printf("gaprole");
  System_flush();

  Sadna_createTask();

  SimpleBLEPeripheral_createTask();

  /* enable interrupts and start SYS/BIOS */
  BIOS_start();

  return 0;
}

/* Pin driver handles */
//static PIN_Handle buttonPinHandle;
//static PIN_Handle ledPinHandle;
//
///* Global memory storage for a PIN_Config table */
//static PIN_State buttonPinState;
//static PIN_State ledPinState;

/*
 * Initial LED pin configuration table
 *   - LEDs Board_PIN_LED0 is on.
 *   - LEDs Board_PIN_LED1 is off.
 */
PIN_Config ledPinTable[] = {
    Board_PIN_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

PIN_Config GPIOTable[] = {
      Board_DIO24_ANALOG  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_OPENDRAIN,
      //Board_PIN_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX, /* LED initially off */
      PIN_TERMINATE
};

//PIN_Config buttonPinTable[] = {
//    Board_PIN_BUTTON0  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
//    Board_PIN_BUTTON1  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_NEGEDGE,
//    PIN_TERMINATE
//};

//void buttonCallbackFxn(PIN_Handle handle, PIN_Id pinId) {
//    uint32_t currVal = 0;
//
//    /* Debounce logic, only toggle if the button is still pushed (low) */
//    CPUdelay(8000*50);
//    if (!PIN_getInputValue(pinId)) {
//        /* Toggle LED based on the button pressed */
//        switch (pinId) {
//            case Board_PIN_BUTTON0:
//                currVal =  PIN_getOutputValue(Board_PIN_LED0);
//                PIN_setOutputValue(ledPinHandle, Board_PIN_LED0, !currVal);
//                PIN_setOutputValue(ledPinHandle, Board_DIO23_ANALOG, !currVal);
//                PIN_setOutputValue(gPin, Board_DIO24_ANALOG, !currVal);
//                break;
//
//            case Board_PIN_BUTTON1:
//                currVal =  PIN_getOutputValue(Board_PIN_LED1);
//                PIN_setOutputValue(ledPinHandle, Board_PIN_LED1, !currVal);
//                break;
//
//            default:
//                /* Do nothing */
//                break;
//        }
//    }
//}

int cell;

int isOpen(int cell){
    ADC_Handle  adc;
    ADC_Params  params;
    uint32_t    res;
    uint32_t    resRaw;
    uint32_t    adcValue;

    ADC_init();
    ADC_Params_init(&params);
    adc = ADC_open(Board_ADC0, &params);
    if (adc != NULL) {
        res = ADC_convert(adc, &adcValue);
        /*if (res == ADC_STATUS_SUCCESS) {
            //use adcValue
        }*/
        resRaw = ADC_convertRawToMicroVolts (adc, res);
        ADC_close(adc);
        if (adcValue < LIMIT){
            // cell is closed - send message to contact list
            return 0;
        }
        else{
            //cell is open and pills have been taken - do nothing
            return 1;
        }
    }
    return -1; // function failure
}

//static void Sadna_fxn(UArg a0, UArg a1)
//{
//    System_printf("entering1\n");
//    System_flush();
//}
static void Sadna_fxn(UArg a0, UArg a1)
{
    System_printf("entering");
    System_flush();
    PIN_State   pinState;
    PIN_State   gpinState;
    PIN_Handle  hPin;
    PIN_Handle  gPin;
    uint32_t    currentOutputVal;
    uint32_t    gcurrentOutputVal;
    uint32_t    standbyDuration = 1;
    uint32_t    currVal = 0;
    bool        taken;
    bool        wasTaken;
    int         i;
    uint32_t    INT0; 
    uint32_t    INT1; 
    uint32_t    INT2; 
    uint32_t    INT3;


    /* Allocate LED pins */
    hPin = PIN_open(&pinState, ledPinTable);
    gPin = PIN_open(&gpinState, GPIOTable);

    cell = 1;

    /*
     * Repeatedly sleeps for a duration, to allow repeated entry/exit
     * from standby. The LED states are toggled on each iteration
     */

//    currentHour = 17;
//    currentMinutes = 40;
//    morningTime = 8;
//    noonTime = 14;
//    eveningTime = 20;
    INT0 = (((morningTime - currentHour + 24)%24)*60 - currentMinutes) * 60;
    INT1 = (((noonTime - morningTime)%24)*60 - 30) * 60;
    INT2 = (((eveningTime - noonTime)%24)*60 - 30) * 60;
    INT3 = (((morningTime - eveningTime + 24)%24)*60 - 30) * 60;
    Task_sleep(INT0*TICKS_PER_SECOND);
    while(1) {
        /* Sleep, to let the power policy transition the device to standby */
        taken = false;
        wasTaken = false;
        /* Read current output value for all pins */
        currentOutputVal =  PIN_getPortOutputValue(hPin);

        /* Toggle the LEDs, configuring all LEDs at once */
        PIN_setPortOutputValue(hPin, ~currentOutputVal);
        for (i = 0; i < SNOOZETIME; i++){
            Task_sleep(standbyDuration*TICKS_PER_SECOND);
            if (isOpen(cell) == 1){
                taken = true;
                if (wasTaken != taken){
                    /* Read current output value for all pins */
                    currentOutputVal =  PIN_getPortOutputValue(hPin);

                    /* Toggle the LEDs, configuring all LEDs at once */
                    PIN_setPortOutputValue(hPin, ~currentOutputVal);
                }
                wasTaken = taken;
            }
        }
        if (taken == false){
            // pills weren't taken - send message to contact list
            gcurrentOutputVal =  PIN_getPortOutputValue(gPin);

            /* Toggle the LEDs, configuring all LEDs at once */
            PIN_setPortOutputValue(gPin, ~gcurrentOutputVal);
//            PIN_setOutputValue(gPin, Board_DIO24_ANALOG, currVal);
            Task_sleep(1*TICKS_PER_SECOND);
//            PIN_setOutputValue(gPin, Board_DIO24_ANALOG, !currVal);
            PIN_setPortOutputValue(gPin, gcurrentOutputVal);

        }
        if (cell % 3 == 1){
            Task_sleep(INT1*TICKS_PER_SECOND);
        }
        if (cell % 3 == 2){
            Task_sleep(INT2*TICKS_PER_SECOND);
        }
        if (cell % 3 == 0){
            Task_sleep(INT3*TICKS_PER_SECOND);
        }
        cell++;
        cell = (cell -1) % 21 + 1;

    }

    /* Open LED pins
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        /* Error initializing board LED pins
        while(1);
    }

    buttonPinHandle = PIN_open(&buttonPinState, buttonPinTable);
    if(!buttonPinHandle) {
        /* Error initializing button pins
        while(1);
    }

    /* Setup callback for button pins
    if (PIN_registerIntCb(buttonPinHandle, &buttonCallbackFxn) != 0) {
        /* Error registering button callback function
        while(1);
    }

    /* Loop forever
    while(1) {
        Task_sleep(1000);
    }*/
}


Task_Struct sbpTask1;
uint8_t sbpTaskStack1[512];


#define SBP_TASK_PRIORITY1                     1

void Sadna_createTask(void)
{
  Task_Params taskParams;
  System_printf("createtak ");
  System_flush();
  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = sbpTaskStack1;
  taskParams.stackSize = 512;
  taskParams.priority = SBP_TASK_PRIORITY1;
  System_printf("construct ");
  System_flush();
  Task_construct(&sbpTask1, Sadna_fxn, &taskParams, NULL);

}


/*******************************************************************************
 * @fn          AssertHandler
 *
 * @brief       This is the Application's callback handler for asserts raised
 *              in the stack.  When EXT_HAL_ASSERT is defined in the Stack
 *              project this function will be called when an assert is raised, 
 *              and can be used to observe or trap a violation from expected 
 *              behavior.       
 *              
 *              As an example, for Heap allocation failures the Stack will raise 
 *              HAL_ASSERT_CAUSE_OUT_OF_MEMORY as the assertCause and 
 *              HAL_ASSERT_SUBCAUSE_NONE as the assertSubcause.  An application
 *              developer could trap any malloc failure on the stack by calling
 *              HAL_ASSERT_SPINLOCK under the matching case.
 *
 *              An application developer is encouraged to extend this function
 *              for use by their own application.  To do this, add hal_assert.c
 *              to your project workspace, the path to hal_assert.h (this can 
 *              be found on the stack side). Asserts are raised by including
 *              hal_assert.h and using macro HAL_ASSERT(cause) to raise an 
 *              assert with argument assertCause.  the assertSubcause may be
 *              optionally set by macro HAL_ASSERT_SET_SUBCAUSE(subCause) prior
 *              to asserting the cause it describes. More information is
 *              available in hal_assert.h.
 *
 * input parameters
 *
 * @param       assertCause    - Assert cause as defined in hal_assert.h.
 * @param       assertSubcause - Optional assert subcause (see hal_assert.h).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void AssertHandler(uint8 assertCause, uint8 assertSubcause)
{
  // Open the display if the app has not already done so
  if ( !dispHandle )
  {
    dispHandle = Display_open(Display_Type_LCD, NULL);
  }

  Display_print0(dispHandle, 0, 0, ">>>STACK ASSERT");

  // check the assert cause
  switch (assertCause)
  {
    case HAL_ASSERT_CAUSE_OUT_OF_MEMORY:
      Display_print0(dispHandle, 0, 0, "***ERROR***");
      Display_print0(dispHandle, 2, 0, ">> OUT OF MEMORY!");
      break;

    case HAL_ASSERT_CAUSE_INTERNAL_ERROR:
      // check the subcause
      if (assertSubcause == HAL_ASSERT_SUBCAUSE_FW_INERNAL_ERROR)
      {
        Display_print0(dispHandle, 0, 0, "***ERROR***");
        Display_print0(dispHandle, 2, 0, ">> INTERNAL FW ERROR!");
      }
      else
      {
        Display_print0(dispHandle, 0, 0, "***ERROR***");
        Display_print0(dispHandle, 2, 0, ">> INTERNAL ERROR!");
      }
      break;

    case HAL_ASSERT_CAUSE_ICALL_ABORT:
      Display_print0(dispHandle, 0, 0, "***ERROR***");
      Display_print0(dispHandle, 2, 0, ">> ICALL ABORT!");
      HAL_ASSERT_SPINLOCK;
      break;

    default:
      Display_print0(dispHandle, 0, 0, "***ERROR***");
      Display_print0(dispHandle, 2, 0, ">> DEFAULT SPINLOCK!");
      HAL_ASSERT_SPINLOCK;
  }

  return;
}


/*******************************************************************************
 * @fn          smallErrorHook
 *
 * @brief       Error handler to be hooked into TI-RTOS.
 *
 * input parameters
 *
 * @param       eb - Pointer to Error Block.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void smallErrorHook(Error_Block *eb)
{
  for (;;);
}

#if defined (CC1350_LAUNCHXL) && defined (POWER_SAVING)
/*******************************************************************************
 * @fn          rFSwitchNotifyCb
 *
 * @brief       Power driver callback to toggle RF switch on Power state
 *              transitions.
 *
 * input parameters
 *
 * @param   eventType - The state change.
 * @param   eventArg  - Not used.
 * @param   clientArg - Not used.
 *
 * @return  Power_NOTIFYDONE to indicate success.
 */
static uint8_t rFSwitchNotifyCb(uint8_t eventType, uint32_t *eventArg,
                                uint32_t *clientArg)
{
  if (eventType == PowerCC26XX_ENTERING_STANDBY)
  {
    // Power down RF Switch
    PIN_setOutputValue(radCtrlHandle, Board_DIO30_SWPWR, 0);
  }
  else if (eventType == PowerCC26XX_AWAKE_STANDBY)
  {
    // Power up RF Switch
    PIN_setOutputValue(radCtrlHandle, Board_DIO30_SWPWR, 1);
  }

  // Notification handled successfully
  return Power_NOTIFYDONE;
}
#endif //CC1350_LAUNCHXL || POWER_SAVING


/*******************************************************************************
 */

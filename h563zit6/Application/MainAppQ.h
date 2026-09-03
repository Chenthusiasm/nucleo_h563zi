/** @file       MainAppQ.h
 *  @brief      MainApp queue header file and interface.
 */

#ifndef MAIN_APP_Q_H_
#define MAIN_APP_Q_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ADCCmdDefines.h"


/* External typedef ------------------------------------------------------------------------------*/

typedef enum {
    MainAppEvent_Sleep,
    MainAppEvent_UserButtonChange,
    MainAppEvent_ADCCmd,
} MainAppEvent_t;

typedef enum {
    ButtonTransition_Released,
    ButtonTransition_Pressed,
} ButtonTransition_t;

typedef union {
    uint32_t            sleepTime_ms;
    ButtonTransition_t  buttonTransition;
    ADCCmd_t            adcCmd;
} MainAppMsgContent_t;

typedef struct {
    MainAppEvent_t      event;      ///< The event type
    MainAppMsgContent_t content;    ///< The content associated with the event
} MainAppMsg_t;


/* External define -------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

bool MainAppQ_Sleep_ms(uint32_t duration_ms);
bool MainAppQ_UserButtonPressed(void);
bool MainAppQ_UserButtonReleased(void);
bool MainAppQ_ADCCmdScratchPad(uint8_t value);
bool MainAppQ_ADCCmdCheckDefaults(void);
bool MainAppQ_ADCCmdInfo(void);


#ifdef __cplusplus
}
#endif

#endif /* MAIN_APP_Q_H_ */

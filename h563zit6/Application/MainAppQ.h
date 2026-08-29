/** @file       MainAppQ.h
 *  @brief      MainApp queue header file and interface.
 */

#ifndef MAIN_APP_Q_H_
#define MAIN_APP_Q_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stddef.h>
#include <stdint.h>


/* External typedef ------------------------------------------------------------------------------*/

typedef enum {
    MainAppEvent_Sleep,
    MainAppEvent_UserButtonChange,
} MainAppEvent_t;

typedef enum {
    ButtonTransition_Released,
    ButtonTransition_Pressed,
} ButtonTransition_t;

typedef union {
    uint32_t            sleepTime_ms;
    ButtonTransition_t  buttonTransition;
} MainAppMsgContent_t;

typedef struct {
    MainAppEvent_t      event;      ///< The event type
    MainAppMsgContent_t content;    ///< The content associated with the event
} MainAppMsg_t;


/* External define -------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

void MainAppQ_Sleep_ms(uint32_t duration_ms);
void MainAppQ_UserButtonPressed(void);
void MainAppQ_UserButtonReleased(void);


#ifdef __cplusplus
}
#endif

#endif /* MAIN_APP_Q_H_ */

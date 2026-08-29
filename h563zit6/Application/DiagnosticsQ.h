/** @file       DiagnosticsQ.h
 *  @brief      Diagnostics queue header file and interface.
 */

#ifndef DIAGNOSTICS_Q_H_
#define DIAGNOSTICS_Q_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stddef.h>
#include <stdint.h>


/* External typedef ------------------------------------------------------------------------------*/

typedef enum {
    DiagSource_MainApp,
    DiagSource_ISR,

    // Add new sources above this line
    DiagSource_printf,
    DiagSource_Other,
} DiagSource_t;

#define DIAG_MSG_MAX_LEN                (256)
typedef struct {
    DiagSource_t    source;                 ///< The source of the message
    uint32_t        timestamp_ticks;        ///< The number of ticks since scheduler start
    size_t          len;                    ///< The length of the message text
    char            text[DIAG_MSG_MAX_LEN]; ///< The formatted message text
} DiagMsg_t;


/* External define -------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

char const* DiagQ_GetSourceStr(DiagSource_t source);
void DiagQ_printf(const char* fmt, ...);
void DiagQ_Log(DiagSource_t source, const char* fmt, ...);
void DiagQ_LogFromISR(DiagSource_t source, const char* fmt, ...);


#ifdef __cplusplus
}
#endif

#endif /* DIAGNOSTICS_Q_H_ */

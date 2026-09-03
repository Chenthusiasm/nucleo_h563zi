/** @file       ADCCmdDefines.h
 *  @brief      ADC command definitions.
 */

#ifndef ADC_CMD_DEFINES_H_
#define ADC_CMD_DEFINES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdint.h>


/* External typedef ------------------------------------------------------------------------------*/

typedef enum {
    ADCCmdSubcmd_scratchpad,
    ADCCmdSubcmd_checkdefaults,
    ADCCmdSubcmd_info,
} ADCCmdSubcmd_t;

typedef union {
    uint8_t scratchpad_val;
} ADCCmdValues_t;

typedef struct {
    ADCCmdSubcmd_t subcmd;
    ADCCmdValues_t values;
} ADCCmd_t;

/* External define -------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* ADC_CMD_DEFINES_H_ */

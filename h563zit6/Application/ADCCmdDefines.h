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
    ADCCmdSubcmd_fifo,

    // add new subcommands before here
    NUMBER_OF_ADCCmdSubcmd,
} ADCCmdSubcmd_t;

typedef enum {
    ADCCmdFIFOParameter_info,
    ADCCmdFIFOParameter_immediate,
    ADCCmdFIFOParameter_event,
    ADCCmdFIFOParameter_autoread,
    ADCCmdFIFOParameter_manualread,
    ADCCmdFIFOParameter_arm,
    ADCCmdFIFOParameter_rearm,
    ADCCmdFIFOParameter_read,

    // add new parameters before here
    NUMBER_OF_ADCCmdFIFOParameter,
} ADCCmdFIFOParameter_t;

typedef struct {
    ADCCmdFIFOParameter_t parameter;
    uint16_t              watermark; // only meaningful when parameter == ADCCmdFIFOParameter_arm
} ADCCmdFIFOValues_t;

typedef union {
    uint8_t            scratchpadValue; // subcmd == ADCCmdSubcmd_info
    ADCCmdFIFOValues_t fifo;            // subcmd == ADCCmdSubcmd_fifo
} ADCCmdValues_t;

typedef struct {
    ADCCmdSubcmd_t subcmd;
    ADCCmdValues_t values;
} ADCCmd_t;

/* External define -------------------------------------------------------------------------------*/

extern char const* ADCCmdSubcmdNames[NUMBER_OF_ADCCmdSubcmd];
extern char const* ADCCmdFIFOParameterNames[NUMBER_OF_ADCCmdFIFOParameter];

/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* ADC_CMD_DEFINES_H_ */

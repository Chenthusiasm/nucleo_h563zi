/**
 * @file    CmdHelper.h
 * @brief   Interface file for the general helper and utility functions.
 */
#ifndef CMD_HELPER_H_
#define CMD_HELPER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>


/* External typedef ------------------------------------------------------------------------------*/


/* Defines ---------------------------------------------------------------------------------------*/


/* External macro --------------------------------------------------------------------------------*/


/* External variables ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief  Parses str as an 8-bit unsigned value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a uint8_t.
 * @return false str was not fully accepted or could not fit in a uint8_t.
 */
bool ParseStringUInt8(char const* str, uint8_t* out);


#ifdef __cplusplus
}
#endif

#endif /* CMD_HELPER_H_ */

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
 * @brief Parses a string as an 8-bit unsigned value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a uint8_t.
 * @return false str was not fully accepted or could not fit in a uint8_t.
 */
bool ParseStringUInt8(char const* str, uint8_t* out);

/**
 * @brief Parses a string as a 16-bit unsigned value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a uint16_t.
 * @return false str was not fully accepted or could not fit in a uint16_t.
 */
bool ParseStringUInt16(char const* str, uint16_t* out);

/**
 * @brief Parses a string as a 32-bit unsigned value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a uint32_t.
 * @return false str was not fully accepted or could not fit in a uint32_t.
 */
bool ParseStringUInt32(char const* str, uint32_t* out);

/**
 * @brief Parses a string as an 8-bit signed value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a int8_t.
 * @return false str was not fully accepted or could not fit in a int8_t.
 */
bool ParseStringInt8(char const* str, int8_t* out);

/**
 * @brief Parses a string as a 16-bit signed value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a int16_t.
 * @return false str was not fully accepted or could not fit in a int16_t.
 */
bool ParseStringInt16(char const* str, int16_t* out);

/**
 * @brief Parses a string as a 32-bit signed value (decimal, or 0x/0-prefixed hex/octal via base 0).
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in a int32_t.
 * @return false str was not fully accepted or could not fit in a int32_t.
 */
bool ParseStringInt32(char const* str, int32_t* out);


#ifdef __cplusplus
}
#endif

#endif /* CMD_HELPER_H_ */

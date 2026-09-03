/**
 * @file    CmdHelper.c
 * @brief   Implementation of the command helper and utility functions.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include "CmdHelper.h"

#include <assert.h>
#include <cstdbool>
#include <cstdint>
#include <cstdlib>


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

bool ParseStringUInt8(char const* str, uint8_t* out) {
    if ((str == nullptr) || (out == nullptr) || (str[0] == '\0')) {
        return false;
    }
    char *end = nullptr;
    unsigned long value = strtoul(str, &end, 0);
    if ((end == str) || (*end != '\0') || (value > 0xFFUL)) {
        return false;
    }
    *out = (uint8_t) value;
    return true;
}

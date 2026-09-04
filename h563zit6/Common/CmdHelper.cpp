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
#include <limits>
#include <type_traits>


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief Parses a string as the specified type (decimal, or 0x/0-prefixed hex/octal via base 0).
 *
 * @note Does not support 64-bit integers or floating point.
 * 
 * @tparam T Type of the integer to convert the string to.
 * @param[in]  str Null-terminated string to parse.
 * @param[out] out Parsed value, only written on success.
 * @return true  str was fully consumed and fit in the specified type.
 * @return false str was not fully accepted or could not fit in the specified type.
 */
template<typename T>
static bool parseStringToInteger(char const* str, T* out) {
    static_assert(!std::is_floating_point_v<T>, "parseStringToInteger() does not support floating point");
    static_assert(!std::is_same<T, uint64_t>::value, "parseStringToInteger() does not support uint64_t");
    static_assert(!std::is_same<T, int64_t>::value, "parseStringToInteger() does not support int64_t");

    if ((str == nullptr) || (out == nullptr) || (str[0] == '\0')) {
        return false;
    }
    constexpr T Min {std::numeric_limits<T>::min()}; // get the min value for the specific type
    constexpr T Max {std::numeric_limits<T>::max()}; // get the max value for the specific type
    char *end = nullptr;
    long long value = strtoll(str, &end, 0);
    if ((end == str) || (*end != '\0') ||
        (value < static_cast<long long>(Min)) ||
        (value > static_cast<long long>(Max))) {
        return false;
    }
    *out = static_cast<T>(value);
    return true;
}


/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

bool ParseStringUInt8(char const* str, uint8_t* out) {
    return parseStringToInteger<uint8_t>(str, out);
}

bool ParseStringUInt16(char const* str, uint16_t* out) {
    return parseStringToInteger<uint16_t>(str, out);
}

bool ParseStringUInt32(char const* str, uint32_t* out) {
    return parseStringToInteger<uint32_t>(str, out);
}

bool ParseStringInt8(char const* str, int8_t* out) {
    return parseStringToInteger<int8_t>(str, out);
}

bool ParseStringInt16(char const* str, int16_t* out) {
    return parseStringToInteger<int16_t>(str, out);
}

bool ParseStringInt32(char const* str, int32_t* out) {
    return parseStringToInteger<int32_t>(str, out);
}

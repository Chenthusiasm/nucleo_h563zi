/**
 * @file    Helper.c
 * @brief   Implementation of the general helper and utility functions.
 * @section Legal Disclaimer
 *      ©2024 Whisker, All rights reserved. All contents of this source file and/or any other
 *      related source files are the explicit property of Whisker. Do not distribute. Do not copy.
 */

/* Includes --------------------------------------------------------------------------------------*/

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal constants ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

/**
 * @brief   Performs a uint32 division but rounds the result instead of truncating.
 * @param[in]   dividend    The unsigned integer getting divided.
 * @param[in]   divisor     The unsigned integer dividing the dividend.
 * @param[in]   round       The rounding correction to apply to the dividend to round the quotient.
 * @note    quotient = dividend / divisor
 * @note    quotient = (dividend + round) / divisor
 * @note    It's possible that if both the dividend and round are too large in value, an overflow
 *          can occur; in such cases, a standard truncated division will be performed.
 * @note    Since this is a static function, it's assumed the divisor != 0 and the calling function
 *          validates this.
 * @return  The quotient, the result of dividing the dividend by the divisor.
 */
static uint32_t UIntDivideWithRound(uint32_t dividend, uint32_t divisor, uint32_t round) {
    uint32_t roundDividend = dividend + round;
    // check if the roundDividend overflowed; if so, perform the standard truncated division
    if ((roundDividend < dividend) || (roundDividend < round)) {
        return (dividend / divisor);
    }
    return (roundDividend / divisor);
}

/* External function prototypes ------------------------------------------------------------------*/


/* External functions ----------------------------------------------------------------------------*/

/**
 * @brief   Performs a uint32 division but rounds the result instead of truncating.
 * @param[in]   dividend    The unsigned integer getting divided.
 * @param[in]   divisor     The unsigned integer dividing the dividend.
 * @note    round = divisor / 2
 * @return  The quotient, the result of dividing the dividend by the divisor.
 */
uint32_t UIntRoundingDivide(uint32_t dividend, uint32_t divisor) {
    assert (divisor != 0u);

    uint32_t round = divisor / 2u;
    return UIntDivideWithRound(dividend, divisor, round);
}


/**
 * @brief   Performs a uint32 division but rounds the result up instead of truncating.
 * @param[in]   dividend    The unsigned integer getting divided.
 * @param[in]   divisor     The unsigned integer dividing the dividend.
 * @note    round = divisor - 1
 * @return  The quotient, the result of dividing the dividend by the divisor.
 */
uint32_t UIntCeilingDivide(uint32_t dividend, uint32_t divisor) {
    assert (divisor != 0u);

    uint32_t round = divisor - 1u;
    return UIntDivideWithRound(dividend, divisor, round);
}


/**
 * @brief   Clears the rightmost (least significant) bit that is set.
 * @param   n   The number to clear the rightmost bit that is set.
 * @return  The number with the rightmost set bit cleared.
 */
uint32_t ClearRightmostSetBit(uint32_t n) {
    return (n & (n - 1u));
}


/**
 * @brief   Checks if an unsigned number is a power of two.
 * @param[in]   n   The number to check.
 * @note    This function is O(1) time and space complexity.
 * @return  If the number is a power of two, true; otherwise, false.
 */
bool IsPowerOfTwo(uint32_t n) {
    return (ClearRightmostSetBit(n) == 0u);
}


/**
 * @brief   Gets the bit position of the rightmost set bit.
 * @param[in]   n   The number to find the rightmost set bit.
 * @note    This function is O(1) space and O(number of bits in n) time complexity.
 * @note    n = 0x0001, return 0
 *          n = 0x0002, return
 * @return  The bit position of the rightmost set bit; if n = 0, return 0.
 */
uint8_t RightmostSetBit(uint32_t n) {
    uint32_t mask = ClearRightmostSetBit(n);
    n ^= mask;
    uint8_t position = 0;
    while (n != 0) {
        n >>= 1u;
        position++;
    }
    return position;
}

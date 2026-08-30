/**
 * @file AD408xRegisters.hpp
 * @brief Configuration register map for the AD408x family (AD4080, AD4083, AD4085).
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <cstdint>

namespace AD408x {

    /**
    * @brief Well-known configuration register addresses.
    */
    enum class Register : uint16_t {
        ChipType            = 0x03, ///< reset value 0x07
        ProductIDL          = 0x04, ///< reset value 0x50 (AD4080) / 0x53 (AD4083)
        ProductIDH          = 0x05, ///< reset value 0x00
        ChipGrade           = 0x06, ///< reset value 0x02
        ScratchPad          = 0x0A, ///< reset value 0x00, R/W
        SPIRevision         = 0x0B, ///< reset value 0x83
    };

    /**
    * @brief Chip Type register (Address 0x03, Reset 0x07).
    */
    struct ChipType {
        static constexpr Register Address = Register::ChipType;

        union Fields {
            uint8_t raw;
            struct {
                uint8_t CHIP_TYPE : 4;    ///< Precision ADC family identifier. Expect 0x7.
                uint8_t RESERVED_7_4 : 4; ///< Reserved. Read-only.
            };
        };
    };

    /**
    * @brief Product ID Low register (Address 0x04, Reset 0x00).
    *
    * Low byte of the 16-bit product ID. This is the field that differs across the AD408x family (0x50 for AD4080, 0x53
    * for AD4083); do not assume a fixed reset value here.
    */
    struct ProductIDL {
        static constexpr Register Address = Register::ProductIDL;

        union Fields {
            uint8_t raw;
            struct {
                uint8_t PRODUCT_ID : 8; ///< Product ID, bits [7:0].
            };
        };
    };

    /**
    * @brief Product ID High register (Address 0x05, Reset 0x00).
    */
    struct ProductIDH {
        static constexpr Register Address = Register::ProductIDH;

        union Fields {
            uint8_t raw;
            struct {
                uint8_t PRODUCT_ID : 8; ///< Product ID, bits [15:8].
            };
        };
    };

    /**
    * @brief Chip Grade register (Address 0x06, Reset 0x02).
    */
    struct ChipGrade {
        static constexpr Register Address = Register::ChipGrade;

        union Fields {
            uint8_t raw;
            struct {
                uint8_t DEVICE_REVISION : 4; ///< Device hardware revision. Expect 0x2.
                uint8_t GRADE : 4;           ///< Device performance grade. May differ across parts/grades.
            };
        };
    };

    /**
    * @brief Scratch Pad register (Address 0x0A, Reset 0x00).
    *
    * No device side effects; used for R/W sanity checks of the CFG bus.
    */
    struct ScratchPad {
        static constexpr Register Address = Register::ScratchPad;

        union Fields {
            uint8_t raw;
            struct {
                uint8_t SCRATCH_VALUE : 8; ///< Software scratchpad value.
            };
        };
    };

}
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
     * @brief Well-known configuration register addresses (Table 31, Address 0x00-0x11 and the FIFO configuration block,
     * Address 0x1C-0x1E).
     *
     * For multibyte registers (currently only FIFO_WATERMARK, EVENT_HYSTERESIS, EVENT_DETECTION_HI, EVENT_DETECTION_LO,
     * OFFSET, and GAIN), the address given here is the MSB byte's address, per the AD408x default
     * (STRICT_REGISTER_ACCESS = 1, ADDR_ASCENSION = 0): a multibyte access addresses the most significant byte and the
     * internal pointer decrements from there.
     */
    enum class Register : uint16_t {
        INTERFACE_CONFIG_A     = 0x00, ///< reset value 0x10
        INTERFACE_CONFIG_B     = 0x01, ///< reset value 0x00
        DEVICE_CONFIG          = 0x02, ///< reset value 0x00
        CHIP_TYPE              = 0x03, ///< reset value 0x07
        PRODUCT_ID_L           = 0x04, ///< reset value 0x50 (AD4080) / 0x53 (AD4083)
        PRODUCT_ID_H           = 0x05, ///< reset value 0x00
        CHIP_GRADE             = 0x06, ///< reset value 0x02
        SCRATCH_PAD            = 0x0A, ///< reset value 0x00, R/W
        SPI_REVISION           = 0x0B, ///< reset value 0x83
        VENDOR_L               = 0x0C, ///< reset value 0x56
        VENDOR_H               = 0x0D, ///< reset value 0x04
        STREAM_MODE            = 0x0E, ///< reset value 0x00
        TRANSFER_CONFIG        = 0x0F, ///< reset value 0x00
        INTERFACE_CONFIG_C     = 0x10, ///< reset value 0x23
        INTERFACE_STATUS_A     = 0x11, ///< reset value 0x00, status bits are R/W1C
        DEVICE_STATUS          = 0x14, ///< reset value 0x09, status bits are R/W1C
        ADC_DATA_INTF_CONFIG_A = 0x15, ///< reset value 0x40
        ADC_DATA_INTF_CONFIG_B = 0x16, ///< reset value 0x00
        ADC_DATA_INTF_CONFIG_C = 0x17, ///< reset value 0x20
        PWR_CTRL               = 0x18, ///< reset value 0x00
        GPIO_CONFIG_A          = 0x19, ///< reset value 0x01
        GPIO_CONFIG_B          = 0x1A, ///< reset value 0x00
        GPIO_CONFIG_C          = 0x1B, ///< reset value 0x00
        GENERAL_CONFIG         = 0x1C, ///< reset value 0x00
        FIFO_WATERMARK         = 0x1E, ///< MSB byte of the 0x1D/0x1E pair, reset value 0x4000
        EVENT_HYSTERESIS       = 0x20, ///< MSB byte of the 0x1F/0x20 pair, reset value 0x0000
        EVENT_DETECTION_HI     = 0x22, ///< MSB byte of the 0x21/0x22 pair, reset value 0x0000
        EVENT_DETECTION_LO     = 0x24, ///< MSB byte of the 0x23/0x24 pair, reset value 0x0000
        OFFSET                 = 0x26, ///< MSB byte of the 0x25/0x26 pair, reset value 0x0000
        GAIN                   = 0x28, ///< MSB byte of the 0x27/0x28 pair, reset value 0x0200
        FILTER_CONFIG          = 0x29, ///< reset value 0x00
    };

    /**
     * @brief Interface Configuration A register (Address 0x00, Reset 0x10).
     *
     * Only SW_RESET, ADDR_ASCENSION, and SW_RESETX are writable; RSVD_6, SDO_ENABLE, and RSVD_3_1 are read-only per the
     * datasheet's Register Details section, so always read before writing to avoid stomping them with a zero-
     * initialized Fields value.
     */
    struct INTERFACE_CONFIG_A {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t SW_RESETX      : 1; ///< Second of the two SW_RESET bits. Both must be set together to reset.
                uint8_t RSVD_3_1       : 3; ///< Reserved. Read-only.
                uint8_t SDO_ENABLE     : 1; ///< SDO pin enable. Read-only.
                uint8_t ADDR_ASCENSION : 1; ///< 0 = address decrements when streaming (default), 1 = increments.
                uint8_t RSVD_6         : 1; ///< Reserved. Read-only.
                uint8_t SW_RESET       : 1; ///< First of the two SW_RESET bits. Both must be set together to reset.
            };
        };

        static constexpr const char* Name    = "INTERFACE_CONFIG_A";
        static constexpr Register    Address = Register::INTERFACE_CONFIG_A;
        static constexpr Fields      Default = { .raw = 0x10 };
    };

    /**
     * @brief Interface Configuration B register (Address 0x01, Reset 0x00).
     */
    struct INTERFACE_CONFIG_B {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t RSVD_2_0          : 3; ///< Reserved. Read-only.
                uint8_t SHORT_INSTRUCTION : 1; ///< 0 = 15-bit addressing (default), 1 = 7-bit addressing.
                uint8_t RSVD_6_4          : 3; ///< Reserved. Read-only.
                uint8_t SINGLE_INST       : 1; ///< 0 = streaming mode (default), 1 = single instruction mode.
            };
        };

        static constexpr const char* Name    = "INTERFACE_CONFIG_B";
        static constexpr Register    Address = Register::INTERFACE_CONFIG_B;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Device Configuration register (Address 0x02, Reset 0x00).
     */
    struct DEVICE_CONFIG {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t OPERATING_MODES : 2; ///< 00 = normal, 10 = standby, 11 = sleep.
                uint8_t RSVD_7_2        : 6; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "DEVICE_CONFIG";
        static constexpr Register    Address = Register::DEVICE_CONFIG;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Chip Type register (Address 0x03, Reset 0x07).
     */
    struct CHIP_TYPE {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t CHIP_TYPE : 4; ///< Precision ADC family identifier. Expect 0x7.
                uint8_t RSVD_7_4  : 4; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "CHIP_TYPE";
        static constexpr Register    Address = Register::CHIP_TYPE;
        static constexpr Fields      Default = { .raw = 0x07 };
    };

    /**
     * @brief Product ID Low register (Address 0x04, Reset 0x00).
     *
     * Low byte of the 16-bit product ID. This is the field that differs across the AD408x family (0x50 for AD4080,
     * 0x53 for AD4083).
     */
    struct PRODUCT_ID_L {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t PRODUCT_ID : 8; ///< Product ID, bits [7:0].
            };
        };

        static constexpr const char* Name    = "PRODUCT_ID_L";
        static constexpr Register    Address = Register::PRODUCT_ID_L;
#ifdef AD4083
        static constexpr Fields      Default = { .raw = 0x53 };
#else
        static constexpr Fields      Default = { .raw = 0x50 };
#endif
    };

    /**
     * @brief Product ID High register (Address 0x05, Reset 0x00).
     */
    struct PRODUCT_ID_H {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t PRODUCT_ID : 8; ///< Product ID, bits [15:8].
            };
        };

        static constexpr const char* Name    = "PRODUCT_ID_H";
        static constexpr Register    Address = Register::PRODUCT_ID_H;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Chip Grade register (Address 0x06, Reset 0x02).
     */
    struct CHIP_GRADE {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t DEVICE_REVISION : 4; ///< Device hardware revision. Expect 0x2.
                uint8_t GRADE           : 4; ///< Device performance grade. May differ across parts/grades.
            };
        };

        static constexpr const char* Name    = "CHIP_GRADE";
        static constexpr Register    Address = Register::CHIP_GRADE;
        static constexpr Fields      Default = { .raw = 0x02 };
    };

    /**
     * @brief Scratch Pad register (Address 0x0A, Reset 0x00).
     *
     * No device side effects; used for R/W sanity checks of the CFG bus.
     */
    struct SCRATCH_PAD {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t SCRATCH_VALUE : 8; ///< Software scratchpad value.
            };
        };

        static constexpr const char* Name    = "SCRATCH_PAD";
        static constexpr Register    Address = Register::SCRATCH_PAD;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief SPI Revision register (Address 0x0B, Reset 0x83).
     */
    struct SPI_REVISION {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t VERSION  : 6; ///< SPI version. 0b000011 = Revision 1.1.
                uint8_t SPI_TYPE : 2; ///< Always reads 0x2.
            };
        };

        static constexpr const char* Name    = "SPI_REVISION";
        static constexpr Register    Address = Register::SPI_REVISION;
        static constexpr Fields      Default = { .raw = 0x83 };
    };

    /**
     * @brief Vendor ID Low register (Address 0x0C, Reset 0x56).
     */
    struct VENDOR_L {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t VID : 8; ///< Analog Devices vendor ID, bits [7:0].
            };
        };

        static constexpr const char* Name    = "VENDOR_L";
        static constexpr Register    Address = Register::VENDOR_L;
        static constexpr Fields      Default = { .raw = 0x56 };
    };

    /**
     * @brief Vendor ID High register (Address 0x0D, Reset 0x04).
     */
    struct VENDOR_H {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t VID : 8; ///< Analog Devices vendor ID, bits [15:8].
            };
        };

        static constexpr const char* Name    = "VENDOR_H";
        static constexpr Register    Address = Register::VENDOR_H;
        static constexpr Fields      Default = { .raw = 0x04 };
    };

    /**
     * @brief Stream Mode register (Address 0x0E, Reset 0x00).
     *
     * Per the datasheet, this mode is not supported.
     */
    struct STREAM_MODE {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t LOOP_COUNT : 8; ///< Data byte count before looping to the start address. 0 disables looping.
            };
        };

        static constexpr const char* Name    = "STREAM_MODE";
        static constexpr Register    Address = Register::STREAM_MODE;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Transfer Configuration register (Address 0x0F, Reset 0x00).
     */
    struct TRANSFER_CONFIG {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t RSVD_1_0               : 2; ///< Reserved. Read-only.
                uint8_t KEEP_STREAM_LENGTH_VAL : 1; ///< When set, the loop counter does not reset on CS rising edge.
                uint8_t RSVD_7_3               : 5; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "TRANSFER_CONFIG";
        static constexpr Register    Address = Register::TRANSFER_CONFIG;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Interface Configuration C register (Address 0x10, Reset 0x23).
     *
     * @note The datasheet's own register-detail table (Table 45) names both the [7:6] and [1:0] fields "CRC_ENABLE";
     * the summary table (Table 31) distinguishes them as CRC_ENABLE and CRC_ENABLEB. CRC_ENABLEB must be written with
     * the bitwise inverse of CRC_ENABLE for CRC to actually enable; naming them differently here is required for this
     * to compile as two distinct fields, and matches the summary table's own naming.
     */
    struct INTERFACE_CONFIG_C {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t CRC_ENABLEB            : 2; ///< Must be written as ~CRC_ENABLE for CRC to enable.
                uint8_t ACTIVE_INTERFACE_MODE  : 2; ///< Active SPI mode. Read-only.
                uint8_t SEND_STATUS            : 1; ///< Enables status transmission on SDO during instruction phase.
                uint8_t STRICT_REGISTER_ACCESS : 1; ///< 0 = normal mode, 1 = multibyte registers require full access.
                uint8_t CRC_ENABLE             : 2; ///< 0b00 = disabled, 0b11 = enabled (see CRC_ENABLEB note above).
            };
        };

        static constexpr const char* Name    = "INTERFACE_CONFIG_C";
        static constexpr Register    Address = Register::INTERFACE_CONFIG_C;
        static constexpr Fields      Default = { .raw = 0x23 };
    };

    /**
     * @brief Interface Status A register (Address 0x11, Reset 0x00).
     *
     * @note All named bits here are R/W1C (write 1 to clear). Do not round-trip a value read from this register back
     * through Write() the way other registers are handled elsewhere in this driver: any bit that read back as 1 would
     * be cleared by writing it back as 1. If you need to clear one flag without disturbing others, construct a fresh
     * Fields with only that bit set, rather than reusing a value from Read().
     */
    struct INTERFACE_STATUS_A {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t ADDRESS_INVALID_ERR         : 1; ///< Set when accessing an address outside the memory map.
                uint8_t REGISTER_PARTIAL_ACCESS_ERR : 1; ///< Set on a partial multibyte access under strict access.
                uint8_t WR_TO_RD_ONLY_REG_ERR       : 1; ///< Set on a write attempt to a read-only register.
                uint8_t CRC_ERR                     : 1; ///< Set on a missing or mismatched interface CRC.
                uint8_t CLOCK_COUNT_ERR             : 1; ///< Set when an incorrect number of clocks is detected.
                uint8_t RSVD_6_5                    : 2; ///< Reserved. Read-only.
                uint8_t NOT_READY_ERR               : 1; ///< Set on an SPI transaction before digital init completes.
            };
        };

        static constexpr const char* Name    = "INTERFACE_STATUS_A";
        static constexpr Register    Address = Register::INTERFACE_STATUS_A;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief Device Status register (Address 0x14, Reset 0x09).
     *
     * @note HI_STATUS, LO_STATUS, POR_ANA_FLAG, ADC_CNV_ERR, and POR_FLAG are R/W1C. See the note on INTERFACE_STATUS_A
     * above; the same read-then-write-back hazard applies here.
     */
    struct DEVICE_STATUS {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t POR_FLAG       : 1; ///< R/W1C. Set on a 1.1 V logic supply POR event.
                uint8_t ROM_CRC_ERR    : 1; ///< Read-only. Set on ROM CRC/ECC failure.
                uint8_t ADC_CNV_ERR    : 1; ///< R/W1C. Set when the minimum tCONV spec is breached.
                uint8_t POR_ANA_FLAG   : 1; ///< R/W1C. Set on an analog POR event.
                uint8_t LO_STATUS      : 1; ///< R/W1C. Set when a low-threshold event is detected.
                uint8_t HI_STATUS      : 1; ///< R/W1C. Set when a high-threshold event is detected.
                uint8_t FIFO_READ_DONE : 1; ///< Read-only.
                uint8_t FIFO_FULL      : 1; ///< Read-only.
            };
        };

        static constexpr const char* Name    = "DEVICE_STATUS";
        static constexpr Register    Address = Register::DEVICE_STATUS;
        static constexpr Fields      Default = { .raw = 0x09 };
    };

    /**
     * @brief ADC Data Interface Configuration A register (Address 0x15, Reset 0x40).
     *
     * Bit 6 is documented as reserved but must always be written as 1 (its reset value) rather than left untouched or
     * written as 0 like a normal reserved bit. Read-modify-write still handles this correctly as long as the value read
     * back from hardware is preserved.
     */
    struct ADC_DATA_INTF_CONFIG_A {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t DATA_INTF_MODE : 1; ///< 0 = read back over LVDS, 1 = read back over SPI data interface.
                uint8_t RSVD_1         : 1; ///< Reserved. Read-only.
                uint8_t SPI_LVDS_LANES : 1; ///< 0 = one lane active, 1 = multiple lanes active.
                uint8_t RSVD_3         : 1; ///< Reserved. Read-only.
                uint8_t INTF_CHK_EN    : 1; ///< Enables fixed test pattern output on the ADC data interface.
                uint8_t RSVD_5         : 1; ///< Reserved. Read-only.
                uint8_t RSVD_6         : 1; ///< Reserved, but must always be written as 1. See note above.
                uint8_t RSVD_7         : 1; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "ADC_DATA_INTF_CONFIG_A";
        static constexpr Register    Address = Register::ADC_DATA_INTF_CONFIG_A;
        static constexpr Fields      Default = { .raw = 0x40 };
    };

    /**
     * @brief ADC Data Interface Configuration B register (Address 0x16, Reset 0x00).
     */
    struct ADC_DATA_INTF_CONFIG_B {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t LVDS_CNV_EN        : 1; ///< 0 = CNV pin in CMOS mode, 1 = CNV pin in LVDS mode.
                uint8_t RSVD_1             : 1; ///< Reserved. Read-only.
                uint8_t LVDS_MNC_EN        : 1; ///< Enables LVDS Manchester encoding (dual lane, FILTER_SEL = 0 only).
                uint8_t LVDS_SELF_CLK_MODE : 1; ///< 0 = echo clock mode, 1 = self clock mode.
                uint8_t LVDS_CNV_CLK_CNT   : 4; ///< LVDS clock edge the MSB of the conversion result is available on.
            };
        };

        static constexpr const char* Name    = "ADC_DATA_INTF_CONFIG_B";
        static constexpr Register    Address = Register::ADC_DATA_INTF_CONFIG_B;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief ADC Data Interface Configuration C register (Address 0x17, Reset 0x20).
     */
    struct ADC_DATA_INTF_CONFIG_C {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t RSVD_3_0        : 4; ///< Reserved. Read-only.
                uint8_t LVDS_VOD        : 3; ///< 0b001/0b010/0b100 = ~185/240/325 mV. Other values reset to ~240 mV.
                uint8_t LVDS_RX_CURRENT : 1; ///< 0 = 1x current, 1 = 2x current.
            };
        };

        static constexpr const char* Name    = "ADC_DATA_INTF_CONFIG_C";
        static constexpr Register    Address = Register::ADC_DATA_INTF_CONFIG_C;
        static constexpr Fields      Default = { .raw = 0x20 };
    };

    /**
     * @brief Power Control register (Address 0x18, Reset 0x00).
     *
     * The datasheet advises against writing to this register at all.
     */
    struct PWR_CTRL {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t INTF_LDO_PD    : 1; ///< Disables the IOVDD LDO. Not recommended to write.
                uint8_t ANA_DIG_LDO_PD : 1; ///< Disables the VDD11 LDO. Not recommended to write.
                uint8_t RSVD_7_2       : 6; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "PWR_CTRL";
        static constexpr Register    Address = Register::PWR_CTRL;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief GPIO Configuration A register (Address 0x19, Reset 0x01).
     */
    struct GPIO_CONFIG_A {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t GPO_0_EN    : 1; ///< 0 = GPIO0 is an input, 1 = GPIO0 is an output.
                uint8_t GPO_1_EN    : 1; ///< 0 = GPIO1 is an input, 1 = GPIO1 is an output.
                uint8_t GPO_2_EN    : 1; ///< 0 = GPIO2 is an input, 1 = GPIO2 is an output.
                uint8_t GPO_3_EN    : 1; ///< 0 = GPIO3 is an input, 1 = GPIO3 is an output.
                uint8_t GPIO_0_DATA : 1; ///< GPIO0 readback or write data.
                uint8_t GPIO_1_DATA : 1; ///< GPIO1 readback or write data.
                uint8_t GPIO_2_DATA : 1; ///< GPIO2 readback or write data.
                uint8_t GPIO_3_DATA : 1; ///< GPIO3 readback or write data.
            };
        };

        static constexpr const char* Name    = "GPIO_CONFIG_A";
        static constexpr Register    Address = Register::GPIO_CONFIG_A;
        static constexpr Fields      Default = { .raw = 0x01 };
    };

    /**
     * @brief GPIO Configuration B register (Address 0x1A, Reset 0x00).
     *
     * GPIO_0_SEL/GPIO_1_SEL select the function each pin outputs: 0x0 = config SPI SDO data, 0x1 = FIFO full flag,
     * 0x2 = FIFO read done flag, 0x3 = filter result ready (active low), 0x4 = high threshold detect, 0x5 = low
     * threshold detect, 0x6 = status alert (active low), 0x7 = GPIO data, 0x8 = filter sync input (active low), 0x9 =
     * external event trigger input for FIFO. 0xA and above are not valid settings.
     */
    struct GPIO_CONFIG_B {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t GPIO_0_SEL : 4; ///< GPIO0 function select. See enumeration above.
                uint8_t GPIO_1_SEL : 4; ///< GPIO1 function select. See enumeration above.
            };
        };

        static constexpr const char* Name    = "GPIO_CONFIG_B";
        static constexpr Register    Address = Register::GPIO_CONFIG_B;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief GPIO Configuration C register (Address 0x1B, Reset 0x00).
     *
     * GPIO_2_SEL/GPIO_3_SEL use the same function encoding as GPIO_0_SEL/GPIO_1_SEL in GPIO_CONFIG_B above.
     */
    struct GPIO_CONFIG_C {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t GPIO_2_SEL : 4; ///< GPIO2 function select. See GPIO_CONFIG_B.
                uint8_t GPIO_3_SEL : 4; ///< GPIO3 function select. See GPIO_CONFIG_B.
            };
        };

        static constexpr const char* Name    = "GPIO_CONFIG_C";
        static constexpr Register    Address = Register::GPIO_CONFIG_C;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief General Configuration register (Address 0x1C, Reset 0x00).
     */
    struct GENERAL_CONFIG {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t FIFO_MODE         : 2; ///< 00 = disabled, 01 = immediate trigger, 10/11 = event trigger modes.
                uint8_t RSVD_3_2          : 2; ///< Reserved. Read-only.
                uint8_t ADC_CNV_ERR_ROUTE : 1; ///< Routes ADC conversion error to the alert pin and status register.
                uint8_t LO_ROUTE          : 1; ///< Routes low detection to the alert pin, status register, and FIFO.
                uint8_t HI_ROUTE          : 1; ///< Routes high detection to the alert pin, status register, and FIFO.
                uint8_t INT_EVENT_EN      : 1; ///< Enables internal event detection on the ADC/filtered result.
            };
        };

        static constexpr const char* Name    = "GENERAL_CONFIG";
        static constexpr Register    Address = Register::GENERAL_CONFIG;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

    /**
     * @brief FIFO Watermark register (Address 0x1D and 0x1E, Reset 0x4000).
     *
     * Number of conversions to capture in the FIFO. Per the datasheet, a written value below 1 clips to 1, above
     * 16384 clips to 16384, and in event-trigger capture mode reading the whole FIFO requires this to be a multiple
     * of four. None of those constraints are enforced here; this struct is a thin mirror of the wire format, not a
     * place for application policy.
     */
    struct FIFO_WATERMARK {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t FIFO_WATERMARK : 15; ///< Number of conversions to capture in the FIFO.
                uint16_t RSVD_15        :  1; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "FIFO_WATERMARK";
        static constexpr Register    Address = Register::FIFO_WATERMARK;
        static constexpr Fields      Default = { .raw = 0x4000 };
    };

    /**
     * @brief Event Detection Hysteresis Configuration register (Address 0x1F and 0x20, Reset 0x0000).
     *
     * Unsigned, LSB = 1.46484 mV. 0x000 = 0 x LSB, 0x7FF = 2047 x LSB.
     */
    struct EVENT_HYSTERESIS {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t HYSTERESIS : 11; ///< Hysteresis value.
                uint16_t RSVD_15_11 :  5; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "EVENT_HYSTERESIS";
        static constexpr Register    Address = Register::EVENT_HYSTERESIS;
        static constexpr Fields      Default = { .raw = 0x0000 };
    };

    /**
     * @brief Event Detection High Threshold Configuration register (Address 0x21 and 0x22, Reset 0x0000).
     *
     * Two's complement, LSB = 1.46484 mV. 0x800 = -2048 x LSB, 0x7FF = +2047 x LSB.
     */
    struct EVENT_DETECTION_HI {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t HI_THRESHOLD : 12; ///< High threshold value.
                uint16_t RSVD_15_12   :  4; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "EVENT_DETECTION_HI";
        static constexpr Register    Address = Register::EVENT_DETECTION_HI;
        static constexpr Fields      Default = { .raw = 0x0000 };
    };

    /**
     * @brief Event Detection Low Threshold Configuration register (Address 0x23 and 0x24, Reset 0x0000).
     *
     * Two's complement, LSB = 1.46484 mV. 0x800 = -2048 x LSB, 0x7FF = +2047 x LSB.
     */
    struct EVENT_DETECTION_LO {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t LO_THRESHOLD : 12; ///< Low threshold value.
                uint16_t RSVD_15_12   :  4; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "EVENT_DETECTION_LO";
        static constexpr Register    Address = Register::EVENT_DETECTION_LO;
        static constexpr Fields      Default = { .raw = 0x0000 };
    };

    /**
     * @brief Offset Correction register (Address 0x25 and 0x26, Reset 0x0000).
     *
     * Two's complement, LSB = 0.00572 mV. 0x800 = -2048 x LSB, 0x7FF = +2047 x LSB.
     */
    struct OFFSET {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t OFFSET     : 12; ///< Offset correction coefficient.
                uint16_t RSVD_15_12 :  4; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "OFFSET";
        static constexpr Register    Address = Register::OFFSET;
        static constexpr Fields      Default = { .raw = 0x0000 };
    };

    /**
     * @brief Gain Correction register (Address 0x27 and 0x28, Reset 0x0200).
     *
     * 0x3FF = system gain of 1.0 + 0.015594. 0x200 = gain correction disabled (lowest latency). 0x001 = system gain
     * of 1.0 - 0.015594.
     */
    struct GAIN {
        union Fields {
            uint16_t raw;
            struct {
                uint16_t GAIN       : 10; ///< Gain correction coefficient.
                uint16_t RSVD_15_10 :  6; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "GAIN";
        static constexpr Register    Address = Register::GAIN;
        static constexpr Fields      Default = { .raw = 0x0200 };
    };

    /**
     * @brief Filter Configuration register (Address 0x29, Reset 0x00).
     *
     * Per the datasheet, changing FILTER_SEL requires a reset via the GPIO pin configured for filter synchronization
     * (FILTER_SYNC) before the first filter result is valid.
     */
    struct FILTER_CONFIG {
        union Fields {
            uint8_t raw;
            struct {
                uint8_t FILTER_SEL    : 2; ///< 00 = disabled, 01 = Sinc1, 10 = Sinc5, 11 = Sinc5 + compensation.
                uint8_t RSVD_2        : 1; ///< Reserved. Read-only.
                uint8_t SINC_DEC_RATE : 4; ///< Sinc decimation factor N. See datasheet Table 62 for the encoding.
                uint8_t RSVD_7        : 1; ///< Reserved. Read-only.
            };
        };

        static constexpr const char* Name    = "FILTER_CONFIG";
        static constexpr Register    Address = Register::FILTER_CONFIG;
        static constexpr Fields      Default = { .raw = 0x00 };
    };

}

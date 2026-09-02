/**
 * @file AD408xConfig.hpp
 * @brief Interface for the AD408x family's configuration SPI (CFG bus).
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "main.h"
#include "AD408xRegisters.hpp"
#include <cstdint>

namespace AD408x {

    /**
     * @brief Driver for the AD408x configuration SPI interface (CFG bus: CS/SCLK/SDI/SDO).
     *
     * Does not touch the DATA bus (DCS/DCLK/SDOx); that will be a separate DataFIFO class in this same namespace once
     * the CFG bus side is bench-validated.
     *
     * Bus notes and assumptions (per AD4080 datasheet, "Digital Interface" section):
     *   - SPI Mode 3 (CPOL = 1, CPHA = 1)
     *   - 16-bit instruction phase: bit 15 = R/W, bits 14:0 = register address (default addressing; SHORT_INSTRUCTION
     *     mode, 7-bit address, not used here)
     *   - R/W bit polarity (1 = read, 0 = write)
     *   - Data phase is byte-oriented (8-bit SPI frames). Most registers are a single byte; a handful (e.g.
     *     FifoWatermark) span two consecutive addresses, MSB at the higher address, and are read/written together in
     *     one SPI frame under the AD408x default interface settings (STRICT_REGISTER_ACCESS = 1, ADDR_ASCENSION = 0).
     *   - NSS is software-controlled (SPI_CS is a plain GPIO, not the peripheral's hardware NSS), per the project's
     *     SPI1 configuration.
     *   - Validated at 15.625MHz (/16 off 250MHz SPI1 kernel clock); /8 (31.25MHz) is unreliable over the Zio
     *     interconnect.
     *   - Per Jan K's guidance set the "Master Keep Io State" to enable to ensure the SPI lines are not hi-Z when idle;
     *     importantly, the SCK line is high which was what the previous PrepareTransactionGroup() hack was doing.
     */
    class Config {
    public:
        /**
         * @brief Associates the driver with an SPI handle and CS GPIO.
         *
         * Purely a member-initializing constructor; does not touch hardware. GPIO/SPI peripheral setup must already
         * have run (CubeMX-generated MX_SPIx_Init() / GPIO init) before calling Init().
         *
         * @param[in] hspi   HAL SPI handle for the CFG bus (software NSS).
         * @param[in] csPort GPIO port of the CS pin.
         * @param[in] csPin  GPIO pin mask of the CS pin.
         */
        constexpr Config(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort, uint16_t csPin) :
            hspi(hspi),
            csPort(csPort),
            csPin(csPin) {};

        /**
         * @brief Idles CS high.
         *
         * Call once, after HAL_SPI_Init()/GPIO init have already run for the CFG bus (mirrors AD4080_Init() in the
         * original C driver, split out from the constructor since it touches hardware state).
         */
        void Init();

        /**
         * @brief Reads a register's content, typed by its Fields union.
         *
         * @tparam Reg Register tag type (e.g. InterfaceConfigA). Must define Address and Fields.
         * @return The register's current content.
         */
        template <typename Reg>
        typename Reg::Fields Read() {
            typename Reg::Fields value;
            static_assert(sizeof(value) == 1 || sizeof(value) == 2, "Only 1- or 2-byte registers are supported");
            if constexpr (sizeof(value) == 1) {
                ReadRegistersRaw(static_cast<uint16_t>(Reg::Address), &value.raw, 1);
            } else {
                uint8_t bytes[2];
                ReadRegistersRaw(static_cast<uint16_t>(Reg::Address), bytes, 2);
                value.raw = static_cast<uint16_t>((static_cast<uint16_t>(bytes[0]) << 8) | bytes[1]);
            }
            return value;
        }

        /**
         * @brief Writes a register's content.
         *
         * @tparam Reg Register tag type (e.g. InterfaceConfigA). Must define Address and Fields.
         * @param[in] value The content to write.
         */
        template <typename Reg>
        void Write(typename Reg::Fields value) {
            static_assert(sizeof(value) == 1 || sizeof(value) == 2, "Only 1- or 2-byte registers are supported");
            if constexpr (sizeof(value) == 1) {
                WriteRegistersRaw(static_cast<uint16_t>(Reg::Address), &value.raw, 1);
            } else {
                uint8_t bytes[2] = {
                    static_cast<uint8_t>(value.raw >> 8),
                    static_cast<uint8_t>(value.raw & 0xFFU),
                };
                WriteRegistersRaw(static_cast<uint16_t>(Reg::Address), bytes, 2);
            }
        }

        /**
         * @brief Reads a register and checks it against its documented reset value, logging a mismatch if found.
         *
         * @tparam Reg Register tag type (e.g. INTERFACE_CONFIG_A). Must define Address, Fields, Default, and Name.
         * @return true if the register's current value matches Reg::Default, false otherwise.
         */
        template <typename Reg>
        bool CheckDefault() {
            typename Reg::Fields value = Read<Reg>();
            if (value.raw == Reg::Default.raw) {
                return true;
            }
            LogMismatch(Reg::Name, value.raw, Reg::Default.raw, sizeof(value) == 2);
            return false;
        }

        /**
         * @brief Reads back CHIP_TYPE/PRODUCT_ID_L/PRODUCT_ID_H/CHIP_GRADE and prints them against their documented
         *        reset values.
         *
         * @return true if CHIP_TYPE and the device revision field of CHIP_GRADE matched what the datasheet says a
         * fresh part should report. PRODUCT_ID and GRADE are logged but not checked, since those legitimately differ
         * across the AD408x family and grades.
         */
        bool VerifyChipID();

        /**
         * @brief Writes a value to SCRATCH_PAD and reads it back, for a basic end-to-end R/W sanity check of the CFG
         *        bus.
         *
         * @param[in] testValue Value to write to SCRATCH_PAD.
         * @return true if the value read back matches testValue.
         */
        bool ScratchPadLoopback(uint8_t testValue = 0xA5);

        /**
         * @brief Reads all the configuration registers and checks if the value matches the reset value.
         *
         * @note This function will falsely indicate that a register doesn't match the reset value if the register was
         * written to prior to being called.
         * 
         * @return The number of configuration registers that didn't match the reset values.
         */
        int TestReadAll();

    private:
        /// R/W bit position in the 16-bit instruction phase (bit 15 = 1 for read).
        static constexpr uint16_t ReadBit = 0x8000U;

        /// Drives CS low (asserts, active-low).
        void CS_Low();

        /// Drives CS high (deasserts, active-low).
        void CS_High();

        /**
         * @brief Reads count contiguous bytes starting at address, in one SPI frame.
         * @param[in] address Starting register address (bits 14:0 of the instruction phase).
         * @param[out] buffer Destination for the count bytes read back.
         * @param[in] count Number of data bytes to read.
         */
        void ReadRegistersRaw(uint16_t address, uint8_t *buffer, uint8_t count);

        /**
         * @brief Writes count contiguous bytes starting at address, in one SPI frame.
         * @param[in] address Starting register address (bits 14:0 of the instruction phase).
         * @param[in] buffer Source of the count bytes to write.
         * @param[in] count Number of data bytes to write.
         */
        void WriteRegistersRaw(uint16_t address, const uint8_t *buffer, uint8_t count);

        /**
         * @brief Logs a register's value against its expected default, formatted for the register's byte width.
         * @param[in] name Register name, for the log line (e.g. Reg::Name).
         * @param[in] actual Value read back from the register.
         * @param[in] expected Expected (default) value for the register.
         * @param[in] isTwoByte true to format actual/expected as 4 hex digits, false for 2 hex digits.
         */
        void LogMismatch(const char *name, uint16_t actual, uint16_t expected, bool isTwoByte);

        SPI_HandleTypeDef*  hspi;   ///< HAL SPI handle for the CFG bus.
        GPIO_TypeDef*       csPort; ///< GPIO port of the CS pin.
        uint16_t            csPin;  ///< GPIO pin mask of the CS pin.
    };

}

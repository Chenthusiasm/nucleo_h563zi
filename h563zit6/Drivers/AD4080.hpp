/**
 * @file AD4080.hpp
 * @brief Interface for the AD4080 ADC.
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once

#include "main.h"
#include <cstdint>

/**
 * @brief Driver for the AD4080's configuration SPI interface (CFG bus: CS/SCLK/SDI/SDO).
 *
 * Does not touch the DATA bus (DCS/DCLK/SDOx), yet. The data bus and FIFO/ conversion readout is after the
 * configuration SPI is more mature..
 *
 * Bus notes and assumptions (per AD4080 datasheet, "Digital Interface" section):
 *   - SPI Mode 3 (CPOL = 1, CPHA = 1)
 *   - 16-bit instruction phase: bit 15 = R/W, bits 14:0 = register address (default addressing; SHORT_INSTRUCTION mode,
 *     7-bit address, not used here)
 *   - R/W bit polarity (1 = read, 0 = write)
 *   - Data phase is byte-oriented (8-bit SPI frames), 1 byte per single-byte register access; this driver only handles
 *     single-byte registers for now but some registers are 2-bytes wide.
 *   - NSS is software-controlled (SPI_CS is a plain GPIO, not the peripheral's hardware NSS), per the project's SPI1
 *     configuration.
 */
class AD4080 {
public:
    /**
     * @brief Associates the driver with an SPI handle and CS GPIO.
     *
     * Purely a member-initializing constructor; does not touch hardware. GPIO/SPI peripheral setup must already have
     * run (CubeMX-generated MX_SPIx_Init() / GPIO init) before calling Init().
     *
     * @param[in] hspi   HAL SPI handle for the CFG bus (software NSS).
     * @param[in] csPort GPIO port of the CS pin.
     * @param[in] csPin  GPIO pin mask of the CS pin.
     */
    constexpr AD4080(SPI_HandleTypeDef *hspi, GPIO_TypeDef *csPort, uint16_t csPin) :
        hspi(hspi),
        csPort(csPort),
        csPin(csPin) {};

    /**
     * @brief Well-known configuration/product-ID registers (Table 31, Address 0x00-0x11).
     *
     * Reset values are documented and make good first-read sanity checks.
     */
    enum class Register : uint16_t {
        ChipType    = 0x03, ///< reset value 0x07
        ProductIDL  = 0x04, ///< reset value 0x50
        ProductIDH  = 0x05, ///< reset value 0x00 -> ProductID = 0x0050
        ChipGrade   = 0x06, ///< reset value 0x02
        ScratchPad  = 0x0A, ///< reset value 0x00, R/W
    };

    /**
     * @brief Idles CS high.
     *
     * Call once, after HAL_SPI_Init()/GPIO init have already run for the CFG bus (mirrors AD4080_Init() in the original
     * C driver, split out from the constructor since it touches hardware state).
     */
    void Init();

    /**
     * @brief Reads a single-byte register.
     *
     * @param[in] address 15-bit register address (bits 14:0 of the instruction phase).
     * @return The register's value.
     */
    uint8_t ReadRegister(uint16_t address);

    /**
     * @brief Reads a single-byte register.
     *
     * @param[in] reg Register enumerator.
     * @return The register's value.
     */
    uint8_t ReadRegister(Register reg) { return ReadRegister((uint16_t) reg); };

    /**
     * @brief Writes a single-byte register.
     *
     * @param[in] address 15-bit register address (bits 14:0 of the instruction phase).
     * @param[in] value   Value to write.
     */
    void WriteRegister(uint16_t address, uint8_t value);

    /**
     * @brief Writes a single-byte register.
     *
     * @param[in] reg   Register enumerator.
     * @param[in] value Value to write.
     */
    void WriteRegister(Register reg, uint8_t value) { WriteRegister((uint16_t) reg, value); };

    /**
     * @brief Reads back CHIP_TYPE/PRODUCT_ID_L/PRODUCT_ID_H/CHIP_GRADE and prints them against their documented reset
     * values.
     *
     * @note Re-initializes and enables the SPI peripheral immediately before this group of transactions. Empirically,
     * without this the first CFG-bus transaction after idle reads back 0x00 (and a write immediately followed by a
     * readback also reads back 0x00). ADI's own reference driver does the same thing ahead of a transaction group,
     * which is this idea came from; root cause is unknown.
     *
     * @return true if all four values matched what the datasheet says a fresh part should report.
     */
    bool VerifyChipID();

    /**
     * @brief Writes a value to SCRATCH_PAD and reads it back, for a basic end-to-end R/W sanity check of the CFG bus.
     *
     * @note Re-initializes and enables the SPI peripheral immediately before this group of transactions, for the same
     * reason as @ref VerifyChipID(); see its docs above.
     *
     * @param[in] testValue Value to write to SCRATCH_PAD.
     * @return true if the value read back matches testValue.
     */
    bool ScratchPadLoopback(uint8_t testValue = 0xA5);

private:
    /// R/W bit position in the 16-bit instruction phase (bit 15 = 1 for read).
    static constexpr uint16_t ReadBit = 0x8000U;

    /// Drives CS low (asserts, active-low).
    void CS_Low();

    /// Drives CS high (deasserts, active-low).
    void CS_High();

    /**
     * @brief Re-initializes and enables the SPI peripheral.
     *
     * Call once before starting a group of CFG-bus transactions. Without this, the first transaction after idle (and
     * the readback half of a write-then-read) reads back 0x00; root cause not otherwise identified; the practice of
     * re-init/enable ahead of a transaction group was picked up from ADI's own reference driver.
     */
    void PrepareTransactionGroup();

    SPI_HandleTypeDef *hspi;   ///< HAL SPI handle for the CFG bus.
    GPIO_TypeDef *csPort;      ///< GPIO port of the CS pin.
    uint16_t csPin;            ///< GPIO pin mask of the CS pin.
};

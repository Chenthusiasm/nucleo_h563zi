#ifndef AD4080_H
#define AD4080_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// Driver for the AD4080's configuration SPI interface (CFG bus: CS/SCLK/SDI/SDO).
// Does not touch the DATA bus (DCS/DCLK/SDOx) — FIFO/conversion readout is out of
// scope for this driver, by design, until the CFG bus is fully validated.
//
// Bus assumptions (per AD4080 datasheet, "Digital Interface" section):
//   - SPI Mode 3 (CPOL = 1, CPHA = 1)
//   - 16-bit instruction phase: bit 15 = R/W, bits 14:0 = register address
//     (default addressing; SHORT_INSTRUCTION mode, 7-bit address, not used here)
//   - R/W bit polarity (1 = read, 0 = write) is INFERRED from Figure 53's "R"/"W"
//     labeling and standard ADI convention — not found stated as an explicit bit
//     value in the datasheet text. First bench read of CHIP_TYPE (expect 0x07) is
//     the sanity check for this assumption; if it comes back as garbage or all
//     0xFF/0x00, polarity is the first thing to flip.
//   - Data phase is byte-oriented (8-bit SPI frames), 1 byte per single-byte
//     register access — this driver only handles single-byte registers for now.
//   - NSS is software-controlled (SPI_CS is a plain GPIO, not the peripheral's
//     hardware NSS), per the project's SPI1 configuration.

// Well-known configuration/product-ID registers (Table 31, Address 0x00-0x11).
// Reset values are documented and make good first-read sanity checks.
#define AD4080_REG_CHIP_TYPE     0x03U  // reset value 0x07
#define AD4080_REG_PRODUCT_ID_L  0x04U  // reset value 0x50
#define AD4080_REG_PRODUCT_ID_H  0x05U  // reset value 0x00 -> ProductID = 0x0050
#define AD4080_REG_CHIP_GRADE    0x06U  // reset value 0x02
#define AD4080_REG_SCRATCH_PAD   0x0AU  // reset value 0x00, R/W

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
} AD4080_Handle;

// Associates the driver with an SPI handle and CS GPIO, and idles CS high.
void AD4080_Init(AD4080_Handle *dev, SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *csPort, uint16_t csPin);

// Reads a single-byte register.
uint8_t AD4080_ReadRegister(AD4080_Handle *dev, uint16_t address);

// Writes a single-byte register.
void AD4080_WriteRegister(AD4080_Handle *dev, uint16_t address, uint8_t value);

// Convenience: reads back CHIP_TYPE/PRODUCT_ID_L/PRODUCT_ID_H/CHIP_GRADE and
// prints them against their documented reset values. Returns true if all four
// matched what the datasheet says a fresh part should report.
bool AD4080_VerifyChipID(AD4080_Handle *dev);

// Convenience: writes a value to SCRATCH_PAD and reads it back, for a basic
// end-to-end R/W sanity check of the CFG bus.
bool AD4080_ScratchPadLoopback(AD4080_Handle *dev, uint8_t testValue);

#ifdef __cplusplus
}
#endif

#endif // AD4080_H

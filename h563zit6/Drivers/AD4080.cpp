/**
 * @file AD4080.cpp
 * @brief Implementation for the AD4080 ADC.
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "AD4080.hpp"

#include "DiagnosticsQ.h"

// Swap this single line to redirect all LOG() calls in this file.
#define LOG(...)                        DiagQ_printf(__VA_ARGS__)
//#define LOG(...)                        DiagQ_Log(DiagSource_AD4080Config, ##__VA_ARGS__)

/**
 * @brief Drives CS low (asserts, active-low).
 */
void AD4080::CS_Low() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
}

/**
 * @brief Drives CS high (deasserts, active-low).
 */
void AD4080::CS_High() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void AD4080::PrepareTransactionGroup() {
    HAL_SPI_Init(hspi);
    __HAL_SPI_ENABLE(hspi);
}

void AD4080::Init() {
    CS_High(); // CS idles high (active low)
}

uint8_t AD4080::ReadRegister(uint16_t address) {
    // Instruction phase (16 bits, MSB first): R/W bit (1) + 15-bit address.
    uint16_t instruction = ReadBit | (address & 0x7FFFU);
    // Data phase: one padding byte. Datasheet recommends driving Logic 1 on SDI during the padding byte of a read
    // access (avoids accidentally looking like a write to address 0 if the bus were ever misinterpreted).
    uint8_t tx[3] = {
        (uint8_t) (instruction >> 8),
        (uint8_t) (instruction & 0xFFU),
        0xFFU,
    };
    uint8_t rx[3] = {0};

    CS_Low();
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx, rx, sizeof(tx), 10);
    //LOG("%s(%04x)=%d\r\n", __func__, address, status);
    (void) status;
    CS_High();

    // The register value comes back during the data-phase byte, i.e. the third byte of the full-duplex exchange.
    return rx[2];
}

void AD4080::WriteRegister(uint16_t address, uint8_t value) {
    // Instruction phase: R/W bit = 0 (write) + 15-bit address.
    uint16_t instruction = address & 0x7FFFU;
    uint8_t tx[3] = {
        (uint8_t) (instruction >> 8),
        (uint8_t) (instruction & 0xFFU),
        value,
    };
    uint8_t rx[3];

    CS_Low();
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx, rx, sizeof(tx), 10);
    //LOG("%s(%04x, %02x)=%d\r\n", __func__, address, value, status);
    (void) status;
    CS_High();
}

bool AD4080::VerifyChipID() {
    PrepareTransactionGroup();

    uint8_t chipType   = ReadRegister(Register::ChipType);
    uint8_t productIdL = ReadRegister(Register::ProductIDL);
    uint8_t productIdH = ReadRegister(Register::ProductIDH);
    uint8_t chipGrade  = ReadRegister(Register::ChipGrade);

    LOG("CHIP_TYPE=0x%02X (expect 0x07)\r\n", chipType);
    LOG("PRODUCT_ID=0x%04X (expect 0x0050)\r\n", static_cast<uint16_t>((productIdH << 8) | productIdL));
    LOG("CHIP_GRADE=0x%02X (expect 0x02)\r\n", chipGrade);

    return (chipType == 0x07U) && (productIdL == 0x50U) && (productIdH == 0x00U) && (chipGrade == 0x02U);
}

bool AD4080::ScratchPadLoopback(uint8_t testValue) {
    PrepareTransactionGroup();

    WriteRegister(Register::ScratchPad, testValue);
    uint8_t readBack = ReadRegister(Register::ScratchPad);

    LOG("SCRATCH_PAD wrote 0x%02X, read back 0x%02X\r\n", testValue, readBack);

    return readBack == testValue;
}

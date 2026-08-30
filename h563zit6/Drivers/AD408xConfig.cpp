/**
 * @file AD408xConfig.cpp
 * @brief Implementation for the AD408x family's configuration SPI (CFG bus).
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "AD408xConfig.hpp"

#include "DiagnosticsQ.h"

// Swap this single line to redirect all LOG() calls in this file:
#define LOG(...)                        DiagQ_printf(__VA_ARGS__)
//#define LOG(...)                        DiagQ_Log(DiagSource_AD408xConfig, ##__VA_ARGS__)

using namespace AD408x;

void Config::CS_Low() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
}

void Config::CS_High() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void Config::PrepareTransactionGroup() {
    HAL_SPI_Init(hspi);
    __HAL_SPI_ENABLE(hspi);
}

void Config::Init() {
    CS_High(); // CS idles high (active low)
}

void Config::ReadRegistersRaw(uint16_t address, uint8_t *buffer, uint8_t count) {
    // instruction phase (16 bits, MSB first): R/W bit (1) + 15-bit address
    // data phase: one padding byte per data byte read
    // datasheet recommends driving Logic 1 on SDI during the padding bytes of a read access (avoids accidentally
    // looking like a write to address 0 if the bus were ever misinterpreted)
    uint16_t instruction = ReadBit | (address & 0x7FFFU);
    uint8_t tx[2 + 2] = {0}; // count is never more than 2 for now, sized for that
    uint8_t rx[2 + 2] = {0};
    tx[0] = (uint8_t) (instruction >> 8);
    tx[1] = (uint8_t) (instruction & 0xFFU);
    for (uint8_t i = 0; i < count; i++) {
        tx[2 + i] = 0xFFU;
    }

    CS_Low();
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx, rx, (uint16_t) (2 + count), 10);
    (void) status; // remove to debug HAL SPI transaction issues
    CS_High();

    // the register values come back during the data-phase bytes, i.e. starting at the third byte of the full-duplex
    // exchange
    for (uint8_t i = 0; i < count; i++) {
        buffer[i] = rx[2 + i];
    }
}

void Config::WriteRegistersRaw(uint16_t address, const uint8_t *buffer, uint8_t count) {
    // Instruction phase: R/W bit = 0 (write) + 15-bit address.
    uint16_t instruction = address & 0x7FFFU;
    uint8_t tx[2 + 2] = {0};
    uint8_t rx[2 + 2];
    tx[0] = (uint8_t) (instruction >> 8);
    tx[1] = (uint8_t) (instruction & 0xFFU);
    for (uint8_t i = 0; i < count; i++) {
        tx[2 + i] = buffer[i];
    }

    CS_Low();
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(hspi, tx, rx, (uint16_t) (2 + count), 10);
    (void) status; // remove to debug HAL SPI transaction issues
    CS_High();
}

bool Config::VerifyChipID() {
    PrepareTransactionGroup();

    ChipType::Fields chipType = ReadRaw<ChipType>();
    ProductIDL::Fields productIdL = ReadRaw<ProductIDL>();
    ProductIDH::Fields productIdH = ReadRaw<ProductIDH>();
    ChipGrade::Fields chipGrade = ReadRaw<ChipGrade>();

    LOG("CHIP_TYPE=0x%02X (expect 0x07)\r\n", chipType.raw);
    LOG("PRODUCT_ID=0x%04X\r\n", static_cast<uint16_t>((productIdH.raw << 8) | productIdL.raw));
    LOG("CHIP_GRADE=0x%02X (device revision expect 0x2)\r\n", chipGrade.raw);

    return (chipType.CHIP_TYPE == 0x7U) && (chipGrade.DEVICE_REVISION == 0x2U);
}

bool Config::ScratchPadLoopback(uint8_t testValue) {
    PrepareTransactionGroup();

    ScratchPad::Fields value;
    value.SCRATCH_VALUE = testValue;
    WriteRaw<ScratchPad>(value);
    ScratchPad::Fields readBack = ReadRaw<ScratchPad>();

    LOG("SCRATCH_PAD wrote 0x%02X, read back 0x%02X\r\n", testValue, readBack.raw);

    return readBack.raw == testValue;
}
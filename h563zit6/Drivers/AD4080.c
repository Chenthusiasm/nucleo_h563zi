#include <DiagnosticsQ.h>
#include "AD4080.h"
#include <stdio.h>


#define AD4080_READ_BIT 0x8000U

static void AD4080_CS_Low(AD4080_Handle *dev) {
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);
}

static void AD4080_CS_High(AD4080_Handle *dev) {
    HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);
}

void AD4080_Init(AD4080_Handle *dev, SPI_HandleTypeDef *hspi,
        GPIO_TypeDef *csPort, uint16_t csPin) {
    dev->hspi = hspi;
    dev->csPort = csPort;
    dev->csPin = csPin;
    AD4080_CS_High(dev); // CS idles high (active low)
}

uint8_t AD4080_ReadRegister(AD4080_Handle *dev, uint16_t address) {
    // Instruction phase (16 bits, MSB first): R/W bit (1) + 15-bit address.
    uint16_t instruction = AD4080_READ_BIT | (address & 0x7FFFU);
    // Data phase: one padding byte. Datasheet recommends driving Logic 1 on SDI
    // during the padding byte of a read access (avoids accidentally looking like
    // a write to address 0 if the bus were ever misinterpreted).
    uint8_t tx[3] = {
            (uint8_t) (instruction >> 8),
            (uint8_t) (instruction & 0xFFU),
            0xFFU,
    };
    uint8_t rx[3] = {0};

    AD4080_CS_Low(dev);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(dev->hspi, tx, rx, sizeof(tx), 10);
    DiagQ_printf("%s(%04x)=%d\r\n", __func__, address, status);
    AD4080_CS_High(dev);

    // The register value comes back during the data-phase byte, i.e. the third
    // byte of the full-duplex exchange.
    return rx[2];
}

void AD4080_WriteRegister(AD4080_Handle *dev, uint16_t address, uint8_t value) {
    // Instruction phase: R/W bit = 0 (write) + 15-bit address.
    uint16_t instruction = address & 0x7FFFU;
    uint8_t tx[3] = {
            (uint8_t) (instruction >> 8),
            (uint8_t) (instruction & 0xFFU),
            value,
    };
    uint8_t rx[3];

    AD4080_CS_Low(dev);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(dev->hspi, tx, rx, sizeof(tx), 10);
    DiagQ_printf("%s(%04x, %02x)=%d\r\n", __func__, address, value, status);
    AD4080_CS_High(dev);
}

bool AD4080_VerifyChipID(AD4080_Handle *dev) {
    uint8_t chipType   = AD4080_ReadRegister(dev, AD4080_REG_CHIP_TYPE);
    //uint8_t chipType2  = AD4080_ReadRegister(dev, AD4080_REG_CHIP_TYPE);
    uint8_t productIdL = AD4080_ReadRegister(dev, AD4080_REG_PRODUCT_ID_L);
    uint8_t productIdH = AD4080_ReadRegister(dev, AD4080_REG_PRODUCT_ID_H);
    uint8_t chipGrade  = AD4080_ReadRegister(dev, AD4080_REG_CHIP_GRADE);

    DiagQ_printf("CHIP_TYPE[1]=0x%02X (expect 0x07)\r\n", chipType);
    //DiagQ_printf("CHIP_TYPE[2]=0x%02X (expect 0x07)\r\n", chipType2);
    DiagQ_printf("PRODUCT_ID=0x%04X (expect 0x0050)\r\n",
            (uint16_t) (productIdH << 8) | productIdL);
    DiagQ_printf("CHIP_GRADE=0x%02X (expect 0x02)\r\n", chipGrade);

    return (chipType == 0x07U) && (productIdL == 0x50U)
            && (productIdH == 0x00U) && (chipGrade == 0x02U);
}

bool AD4080_ScratchPadLoopback(AD4080_Handle *dev, uint8_t testValue) {
    AD4080_WriteRegister(dev, AD4080_REG_SCRATCH_PAD, testValue);
    //AD4080_WriteRegister(dev, AD4080_REG_SCRATCH_PAD, testValue);
    uint8_t readBack = AD4080_ReadRegister(dev, AD4080_REG_SCRATCH_PAD);
    //uint8_t readBack2 = AD4080_ReadRegister(dev, AD4080_REG_SCRATCH_PAD);

    DiagQ_printf("SCRATCH_PAD[1] wrote 0x%02X, read back 0x%02X\r\n", testValue, readBack);
    //DiagQ_printf("SCRATCH_PAD[2] wrote 0x%02X, read back 0x%02X\r\n", testValue, readBack2);

    return readBack == testValue;
}

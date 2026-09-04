/**
 * @file AD408xDataFIFO.cpp
 * @brief Implementation of the AD408x family's FIFO based data readout base class.
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "AD408xDataFIFO.hpp"

#include "AD408xRegisters.hpp"

using namespace AD408x;

void DataFIFO::CS_Low() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
}

void DataFIFO::CS_High() {
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void DataFIFO::InitTrigger() {
    // Immediate trigger mode needs nothing beyond the shared FIFO_FULL setup Init() already does.
}

void DataFIFO::OnArm() {
    // Immediate trigger mode needs nothing beyond the shared WATERMARK/FIFO_MODE writes Arm() already does.
}

void DataFIFO::LockSharedBus() const {
    if (sharedBus) {
        __disable_irq();
    }
}

void DataFIFO::UnlockSharedBus() const {
    if (sharedBus) {
        __enable_irq();
    }
}

void DataFIFO::Init() {
    CS_High(); // DCS idles high (active low)

    LockSharedBus();
    // FIFO_FULL is wired to AD408x GPIO3 on this board revision. Read-modify-write since GPIO_CONFIG_A and
    // GPIO_CONFIG_C are shared registers covering all four GPIOs.
    GPIO_CONFIG_A::Fields gpioA = cfg->Read<GPIO_CONFIG_A>();
    gpioA.GPO_3_EN = 1; // AD408x drives GPIO3 as an output; FIFO_FULL is an AD408x output
    cfg->Write<GPIO_CONFIG_A>(gpioA);

    GPIO_CONFIG_C::Fields gpioC = cfg->Read<GPIO_CONFIG_C>();
    gpioC.GPIO_3_SEL = 0b0001; // 0x1 = FIFO full flag, per the GPIO_CONFIG_B/C function select table
    cfg->Write<GPIO_CONFIG_C>(gpioC);

    InitTrigger();
    UnlockSharedBus();
}

void DataFIFO::Arm(uint16_t count, uint8_t *buffer) {
    rxBuffer = buffer;
    armedCount = count;
    dmaComplete = false;

    LockSharedBus();
    FIFO_WATERMARK::Fields watermark;
    watermark.raw = count;
    cfg->Write<FIFO_WATERMARK>(watermark);

    GENERAL_CONFIG::Fields generalConfig = cfg->Read<GENERAL_CONFIG>();
    generalConfig.FIFO_MODE = ModeValue();
    cfg->Write<GENERAL_CONFIG>(generalConfig);
    UnlockSharedBus();

    OnArm();
}

void DataFIFO::Rearm(uint8_t *buffer) {
    rxBuffer = buffer;
    dmaComplete = false;

    LockSharedBus();
    // Disable then re-enable, per the datasheet's documented rearm sequence. Two separate CFG bus writes: the AD408x
    // needs to see FIFO_MODE actually pass through 0x0 before the second write takes effect. FIFO_WATERMARK is not
    // rewritten here, armedCount (and therefore the DMA burst length in OnFIFOFullISR()) is unchanged from the last
    // Arm() call, only the destination buffer moves.
    GENERAL_CONFIG::Fields generalConfig = cfg->Read<GENERAL_CONFIG>();
    generalConfig.FIFO_MODE = 0x0;
    cfg->Write<GENERAL_CONFIG>(generalConfig);
    generalConfig.FIFO_MODE = ModeValue();
    cfg->Write<GENERAL_CONFIG>(generalConfig);
    UnlockSharedBus();
}

void DataFIFO::OnFIFOFullISR() {
    CS_Low();
    HAL_StatusTypeDef status = HAL_SPI_Receive_DMA(hspiData, rxBuffer,
                                                   static_cast<uint16_t>(armedCount * BytesPerSample));
    (void) status; // remove to debug HAL SPI DMA start issues
}

void DataFIFO::OnDMACompleteISR() {
    CS_High();
    dmaComplete = true;
}

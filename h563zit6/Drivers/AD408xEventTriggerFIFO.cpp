/**
 * @file AD408xEventTriggerFIFO.cpp
 * @brief Implementation of AD408x event trigger, read latest WATERMARK mode FIFO readout.
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "AD408xEventTriggerFIFO.hpp"

#include "AD408xRegisters.hpp"

using namespace AD408x;

uint8_t EventTriggerFIFO::ModeValue() const {
    return 0x2; // datasheet's binary 10, event trigger capture, read latest WATERMARK
}

void EventTriggerFIFO::InitTrigger() {
    // GPIO1 is wired to FIFO_START (EXT_EVENT) on this board revision. Read-modify-write since GPIO_CONFIG_A and
    // GPIO_CONFIG_B are shared registers covering all four GPIOs.
    GPIO_CONFIG_A::Fields gpioA = cfg->Read<GPIO_CONFIG_A>();
    gpioA.GPO_1_EN = 0; // AD408x treats GPIO1 as an input; EXT_EVENT is an AD408x input
    cfg->Write<GPIO_CONFIG_A>(gpioA);

    GPIO_CONFIG_B::Fields gpioB = cfg->Read<GPIO_CONFIG_B>();
    gpioB.GPIO_1_SEL = 0b1001; // 0x9 = external event trigger input, per the GPIO_CONFIG_B function select table
    cfg->Write<GPIO_CONFIG_B>(gpioB);
}

void EventTriggerFIFO::TriggerEvent(bool assert) {
    HAL_GPIO_WritePin(fifoStartPort, fifoStartPin, assert ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

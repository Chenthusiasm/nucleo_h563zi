/**
 * @file AD408xImmediateTriggerFIFO.cpp
 * @brief Implementation of AD408x immediate trigger mode FIFO readout.
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "AD408xImmediateTriggerFIFO.hpp"

using namespace AD408x;

uint8_t ImmediateTriggerFIFO::ModeValue() const {
    return 0x1; // datasheet's binary 01, immediate trigger mode
}

/**
 * @file AD408xImmediateTriggerFIFO.hpp
 * @brief AD408x FIFO readout in immediate trigger mode (FIFO_MODE = 0x1).
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "AD408xDataFIFO.hpp"

namespace AD408x {

    /**
     * @brief FIFO readout that starts capturing as soon as Arm() writes FIFO_MODE, per the AD408x's immediate trigger
     *        mode.
     *
     * No GPIO beyond the FIFO_FULL pin shared by every trigger mode is needed here; InitTrigger() and OnArm() both fall
     * back to DataFIFO's no-op defaults.
     */
    class ImmediateTriggerFIFO: public DataFIFO {
    public:
        using DataFIFO::DataFIFO;

    protected:
        /// @return 0x1, the datasheet's FIFO_MODE value for immediate trigger mode.
        uint8_t ModeValue() const override;
    };
    
}

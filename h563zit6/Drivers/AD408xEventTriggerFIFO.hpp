/**
 * @file AD408xEventTriggerFIFO.hpp
 * @brief AD408x FIFO readout in event trigger mode, read latest WATERMARK (FIFO_MODE = 0x2).
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "AD408xDataFIFO.hpp"

namespace AD408x {

    /**
     * @brief FIFO readout triggered by a level the MCU drives onto the AD408x's FIFO_START (EXT_EVENT) input.
     *
     * Adds a FIFO_START GPIO (MCU output, AD408x input) to the FIFO_FULL GPIO shared with every trigger mode.
     * InitTrigger() configures the AD408x's GPIO1 for the EXT_EVENT function; TriggerEvent() drives the MCU side of
     * that same signal.
     *
     * Per the datasheet, EXT_EVENT is level triggered, not edge triggered: the event fires when a logic high is
     * detected on the configured GPIO input. TriggerEvent(true) should be called only after Arm() has already written
     * FIFO_MODE, and should be followed by TriggerEvent(false) once the burst is known to have started, so the pin is
     * not left high across a later Rearm().
     */
    class EventTriggerFIFO: public DataFIFO {
    public:
        /**
         * @brief Associates the driver with the DATA bus SPI/CS, the shared FIFO_FULL GPIO, the FIFO_START GPIO, and
         *        the Config instance used for CFG bus register access.
         *
         * Purely a member-initializing constructor; does not touch hardware. See DataFIFO's constructor for the shared
         * parameters this one passes through unchanged.
         *
         * @param[in] cfg           Config instance for the CFG bus this ADC shares with the DATA bus below.
         * @param[in] hspiData      HAL SPI handle for the DATA bus (software NSS).
         * @param[in] csPort        GPIO port of the DATA bus chip select (DCS) pin.
         * @param[in] csPin         GPIO pin mask of the DATA bus chip select (DCS) pin.
         * @param[in] fifoFullPort  GPIO port of the FIFO_FULL input pin.
         * @param[in] fifoFullPin   GPIO pin mask of the FIFO_FULL input pin.
         * @param[in] fifoStartPort GPIO port of the FIFO_START (EXT_EVENT) output pin.
         * @param[in] fifoStartPin  GPIO pin mask of the FIFO_START (EXT_EVENT) output pin.
         */
        constexpr EventTriggerFIFO(Config *cfg, SPI_HandleTypeDef *hspiData, GPIO_TypeDef *csPort, uint16_t csPin,
                                   GPIO_TypeDef *fifoFullPort, uint16_t fifoFullPin,
                                   GPIO_TypeDef *fifoStartPort, uint16_t fifoStartPin) :
            DataFIFO(cfg, hspiData, csPort, csPin, fifoFullPort, fifoFullPin),
            fifoStartPort(fifoStartPort),
            fifoStartPin(fifoStartPin) {};

        /**
         * @brief Drives the FIFO_START (EXT_EVENT) pin the MCU owns.
         *
         * @param[in] assert true to drive the pin high, which the AD408x treats as the trigger event; false to drive it
         *                   back low.
         */
        void TriggerEvent(bool assert);

    protected:
        /// @return 0x2, the datasheet's FIFO_MODE value for event trigger capture, read latest WATERMARK.
        uint8_t ModeValue() const override;

        /// Configures the AD408x's GPIO1 as an EXT_EVENT input (GPIO_CONFIG_A/B).
        void InitTrigger() override;

    private:
        GPIO_TypeDef    *fifoStartPort; ///< GPIO port of the FIFO_START (EXT_EVENT) output pin.
        uint16_t        fifoStartPin;   ///< GPIO pin mask of the FIFO_START (EXT_EVENT) output pin.
    };

}

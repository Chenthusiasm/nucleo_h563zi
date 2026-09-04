/**
 * @file AD408xDataFIFO.hpp
 * @brief Base class for the AD408x family's FIFO based data readout, independent of trigger mode.
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "main.h"
#include "AD408xConfig.hpp"
#include "AD408xRegisters.hpp"
#include <cstdint>

// Remove the comment to support 20-bit samples (3-byte sample sizes); this is in the hpp file for now.
#define AD4080

namespace AD408x {

    /**
     * @brief Shared FIFO and DMA machinery for AD408x data readout, independent of trigger mode.
     *
     * Owns the DATA bus SPI, its chip select, and the FIFO_FULL GPIO, all common to every trigger mode this project
     * uses (immediate trigger and event trigger, read latest WATERMARK). Derived classes supply only the FIFO_MODE
     * value and any trigger specific setup through the protected hooks below (non-virtual interface pattern); the
     * public Init()/Arm()/Rearm() sequence itself lives here and is not overridden.
     *
     * Does not touch the CFG bus directly; register access goes through the Config instance supplied at construction,
     * the same instance the rest of the application uses for CFG bus reads and writes.
     *
     * hspi and cfg's own SPI handle may or may not be the same physical peripheral, depending on the board: on the
     * EVAL-AD4080ARDZ bring-up setup they are (one SPI1, software NSS on SPI_CS versus DATA_CS); on the RSG Rev A board
     * they are not. This class detects which case it is in by comparing hspi against cfg->GetHSPI() at construction
     * (see sharedBus below), and only pays for the difference when it matters.
     *
     * Application code is responsible for dispatching two HAL callbacks this class depends on:
     *   - HAL_GPIO_EXTI_Callback(), for the pin matching fifoFullPin, must call OnFIFOFullISR().
     *   - HAL_SPI_RxCpltCallback(), for the hspi matching hspiData, must call OnDMACompleteISR().
     */
    class DataFIFO {
    public:
        /**
         * @brief Associates the driver with the DATA bus SPI/CS, the shared FIFO_FULL GPIO, and the Config instance
         *        used for CFG bus register access.
         *
         * Purely a member-initializing constructor; does not touch hardware. GPIO/SPI peripheral setup must already
         * have run (CubeMX-generated MX_SPIx_Init() / GPIO init) before calling Init().
         *
         * @param[in] cfg          Config instance for the CFG bus this ADC shares with the DATA bus below.
         * @param[in] hspiData     HAL SPI handle for the DATA bus (software NSS).
         * @param[in] csPort       GPIO port of the DATA bus chip select (DCS) pin.
         * @param[in] csPin        GPIO pin mask of the DATA bus chip select (DCS) pin.
         * @param[in] fifoFullPort GPIO port of the FIFO_FULL input pin.
         * @param[in] fifoFullPin  GPIO pin mask of the FIFO_FULL input pin.
         */
        constexpr DataFIFO(Config *cfg, SPI_HandleTypeDef *hspiData, GPIO_TypeDef *csPort, uint16_t csPin,
                            GPIO_TypeDef *fifoFullPort, uint16_t fifoFullPin) :
            cfg(cfg),
            hspiData(hspiData),
            csPort(csPort),
            csPin(csPin),
            fifoFullPort(fifoFullPort),
            fifoFullPin(fifoFullPin),
            sharedBus(hspiData == cfg->GetHSPI()),
            rxBuffer(nullptr),
            armedCount(0),
            dmaComplete(false) {};

        virtual ~DataFIFO() = default;

        /**
         * @brief Configures FIFO_FULL on the AD408x (GPIO_CONFIG_A/C) and calls InitTrigger() for anything the trigger
         *        mode needs beyond that.
         *
         * Call once, after HAL_SPI_Init()/GPIO init have already run for the DATA bus and the FIFO_FULL GPIO.
         */
        void Init();

        /**
         * @brief Writes FIFO_WATERMARK, then arms the FIFO by writing ModeValue() to FIFO_MODE. Calls OnArm() afterward
         *        for any trigger specific follow-up.
         *
         * @param[in] count  Number of conversions to capture, written to FIFO_WATERMARK. Not clamped or validated
         *                   against the datasheet's 1 to 16384 range; callers are responsible for a sane value.
         * @param[in] buffer Destination for the DMA burst read once FIFO_FULL fires. Must remain valid until
         *                   DataReady() returns true, and must be at least 3 * count bytes.
         */
        void Arm(uint16_t count, uint8_t *buffer);

        /**
         * @brief Disables then re-enables the FIFO to rearm it for another capture at the same trigger mode.
         *
         * Shared by every trigger mode: FIFO_MODE = 0x0 (disabled), followed by FIFO_MODE = ModeValue() again. Per the
         * datasheet this is the same sequence documented for immediate trigger mode rearming. Whether it is safe to
         * call while a previous burst is still unread is an open question flagged in the project's design doc, and
         * should be bench verified before relying on it mid sweep.
         */
        void Rearm();

        /**
         * @brief Reports whether the last armed capture has finished.
         * @return true once the FIFO_FULL EXTI has fired and the resulting DMA burst has completed.
         */
        bool DataReady() const {
            return dmaComplete;
        }

        /**
         * @brief Call from the application's HAL_GPIO_EXTI_Callback() dispatch for fifoFullPin.
         *
         * Asserts the DATA bus chip select and starts the DMA burst read of armedCount samples into the buffer that was
         * supplied to Arm().
         */
        void OnFIFOFullISR();

        /**
         * @brief Call from the application's HAL_SPI_RxCpltCallback() dispatch for hspiData.
         *
         * De-asserts the DATA bus chip select and marks the capture as ready. After this call, DataReady() returns true
         * until the next Arm().
         */
        void OnDMACompleteISR();

#if defined(AD4080)
        static constexpr uint8_t BytesPerSample = 3u; // 20-bit resolution
#else
        static constexpr uint8_t BytesPerSample = 2u; // 16-bit resolution, pending bench confirmation
#endif

    protected:
        /**
         * @brief Returns the FIFO_MODE value this trigger mode arms with.
         *
         * @note The datasheet gives FIFO_MODE values as two-bit binary patterns (00, 01, 10, 11), which as decimal/hex
         * values are 0x0 through 0x3. Do not confuse the datasheet's "10" (binary, value 0x2) with hex 0x10.
         *
         * @return The FIFO_MODE value, in the low two bits of the returned byte.
         */
        virtual uint8_t ModeValue() const = 0;

        /**
         * @brief Hook for trigger mode specific setup beyond the shared FIFO_FULL configuration done in Init().
         *
         * Default implementation does nothing. EventTriggeredFIFO overrides this to configure the FIFO_START GPIO as an
         * EXT_EVENT input.
         */
        virtual void InitTrigger();

        /**
         * @brief Hook for anything a trigger mode needs to do right after Arm() writes FIFO_WATERMARK and FIFO_MODE.
         *
         * Default implementation does nothing.
         */
        virtual void OnArm();

        /// Drives the DATA bus chip select (DCS) low (asserts, active low).
        void CS_Low();

        /// Drives the DATA bus chip select (DCS) high (deasserts, active low).
        void CS_High();

        /**
         * @brief Enters a critical section around a CFG bus access, only if sharedBus is true.
         *
         * @note Uses __disable_irq(), matching the short ISR-safe critical sections already used elsewhere in this
         * codebase (see usb.c), rather than a FreeRTOS semaphore: the contention this guards against is a task context
         * CFG bus write versus the FIFO_FULL EXTI ISR, which cannot block waiting on a semaphore. No-op when sharedBus
         * is false, since two separate peripherals cannot contend for each other.
         */
        void LockSharedBus() const;

        /// Leaves the critical section entered by LockSharedBus(). No-op when sharedBus is false.
        void UnlockSharedBus() const;

        Config              *cfg;           ///< Config instance used for CFG bus register access.
        SPI_HandleTypeDef   *hspiData;      ///< HAL SPI handle for the DATA bus.
        GPIO_TypeDef        *csPort;        ///< GPIO port of the DATA bus chip select (DCS) pin.
        uint16_t            csPin;          ///< GPIO pin mask of the DATA bus chip select (DCS) pin.
        GPIO_TypeDef        *fifoFullPort;  ///< GPIO port of the FIFO_FULL input pin.
        uint16_t            fifoFullPin;    ///< GPIO pin mask of the FIFO_FULL input pin.
        const bool          sharedBus;      ///< True if hspiData and cfg->GetHSPI() are the same peripheral.
        uint8_t             *rxBuffer;      ///< Destination buffer for the current armed capture, set by Arm().
        uint16_t            armedCount;     ///< Number of conversions armed for the current capture, set by Arm().
        volatile bool       dmaComplete;    ///< Set by OnDMACompleteISR(), cleared by Arm(). Backs DataReady().
    };
    
}

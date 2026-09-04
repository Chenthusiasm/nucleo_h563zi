/**
 * @file AD408xManager.hpp
 * @brief High-order test interface encapsulating Config, ImmediateTriggerFIFO, EventTriggerFIFO, the FIFO
 *        mode/read-mode/arm-rearm-read state machine, and the sample receive buffer for one AD408x part.
 *
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include "AD408xConfig.hpp"
#include "AD408xImmediateTriggerFIFO.hpp"
#include "AD408xEventTriggerFIFO.hpp"

#include <cstddef>
#include <cstdint>

// Comment this out once the board carries more than one AD408x part; RegisterAsSingleton()/Instance() only make sense
// while there is exactly one Manager instance to find. See RegisterAsSingleton()'s doc comment.
#define AD408X_MANAGER_SINGLETON

namespace AD408x {

    /**
     * @brief Encapsulates one AD408x part's CFG bus, DATA bus (both trigger modes), and FIFO capture bookkeeping behind
     *        a small set of high-order operations, so application code (MainAppTask) calls e.g. ArmFIFO() instead of
     *        reaching into Config/DataFIFO subclasses directly.
     *
     * Owns a Config, an ImmediateTriggerFIFO, and an EventTriggerFIFO (all three constructed from the pin/peripheral
     * mapping passed to Manager's constructor), plus a single receive buffer shared by both FIFO objects and sized to
     * hold RxBufferMaxSamples samples' worth of pending, unread captures.
     *
     * Capture pool design: unlike a single buffer that gets reused (and overwritten) by every Arm()/Rearm(), this class
     * treats rxBuffer as a pool that each accepted ArmFIFO()/RearmFIFO() call claims a fresh, non-overlapping region
     * of, tracked in pendingCaptures[]. This lets several captures accumulate back-to-back, unread, so their contents
     * can be inspected independently once ReadFIFO() (or an automatic read, see SetReadMode()) drains them, e.g. to
     * check whether the AD408x discards/corrupts earlier captures if they are not read out before the next arm/rearm.
     *
     * Only one capture may be in flight (armed, not yet DataReady()) at a time; ArmFIFO()/RearmFIFO()/SetTriggerMode()/
     * SetReadMode() all reject with Result::CaptureInFlight while one is outstanding. Unread-but-completed captures DO
     * NOT block those calls; accumulating unread captures is the point of the pool design above.
     *
     * Thread-safety / ownership model: exactly one task is expected to ever call the mutating methods on a given
     * Manager instance (MainAppTask, via MainAppQ, in this project); there is no internal locking against concurrent
     * callers from multiple tasks. OnFIFOFullISR()/OnDMACompleteISR() are ISR-context safe (cheap bookkeeping only, no
     * CFG bus access, matching the same contract DataFIFO's own ISR hooks already have). PollFIFO() must be called
     * periodically from task context (see its doc comment) to service completed captures.
     *
     * Read-only access from other tasks (e.g. a CLI command querying FIFO state without owning the instance) is
     * supported via the optional singleton accessor below, which deliberately returns a const reference: every mutating
     * method here is non-const, so code holding only the const Manager& from Instance() cannot compile a call to any of
     * them. Only RegisterAsSingleton()'s caller (the actual owner, holding a non-const Manager&) retains mutation
     * rights.
     */
    class Manager {
    public:
        /// Sample capacity of rxBuffer, matching the AD4080 family's documented 16K-sample FIFO depth.
        static constexpr size_t RxBufferMaxSamples = 16000u;

        /// Fixed bound on how many distinct unread captures can be tracked at once in pendingCaptures[]. In practice
        /// this would require 64 manual ArmFIFO()/RearmFIFO() calls without an intervening ReadFIFO(), so it is not
        /// expected to be a real constraint, just a fixed-size array instead of dynamic allocation.
        static constexpr size_t MaxPendingCaptures = 64u;

        /// Watermark RearmFIFO() uses if called before any ArmFIFO() has ever succeeded.
        static constexpr uint16_t DefaultWatermark = 100u;

        /// The name of the class; used for logging.
        static constexpr char const * Name = "AD408x::Manager";

        /// Which DataFIFO subclass ArmFIFO()/RearmFIFO() currently apply to.
        enum class TriggerMode {
            Immediate, ///< ImmediateTriggerFIFO (FIFO_MODE = 0x1).
            Event,     ///< EventTriggerFIFO (FIFO_MODE = 0x2).
        };

        /// Whether a completed capture is drained automatically or left for an explicit ReadFIFO() call.
        enum class ReadMode {
            Automatic, ///< PollFIFO() drains and logs a capture as soon as it completes.
            Manual,    ///< PollFIFO() leaves completed captures pending; only ReadFIFO() drains them.
        };

        /// Result of a state-changing FIFO operation; distinguishes why a request was rejected so the caller (via
        /// MainAppQ/MainAppTask in this project) can log something specific instead of a generic failure.
        enum class Result {
            Ok,                        ///< Request accepted and applied.
            CaptureInFlight,           ///< Armed, waiting on FIFO_FULL/OnDMACompleteISR(); can't touch FIFO_MODE now.
            InvalidWatermark,          ///< watermark == 0, or watermark > RxBufferMaxSamples outright.
            InsufficientBufferSpace,   ///< Valid watermark, but does not fit on top of already-pending captures.
            MaxPendingCapturesReached, ///< pendingCaptureCount == MaxPendingCaptures; must ReadFIFO() first.
            NothingPending,            ///< ReadFIFO() called with no completed, unread captures to drain.
        };

        /// Point-in-time snapshot of Manager's state, returned by GetFIFOInfo(). Plain data, safe to copy out to
        /// another task without any additional synchronization beyond GetFIFOInfo() itself being const-callable.
        struct FIFOInfo {
            TriggerMode triggerMode;            ///< Currently selected trigger mode.
            ReadMode    readMode;               ///< Currently selected read mode.
            bool        captureInFlight;        ///< True if a capture is armed and not yet complete.
            size_t      pendingCaptureCount;    ///< Number of completed, unread captures currently held in the buffer.
            size_t      pendingSampleCount;     ///< Total samples across all pending, unread captures.
            uint16_t    lastWatermark;          ///< Watermark used by the most recent ArmFIFO(), or DefaultWatermark if
                                                ///< ArmFIFO() has never succeeded.
            size_t      armCount;               ///< Number of ArmFIFO() calls that returned Result::Ok.
            size_t      rearmCount;             ///< Number of RearmFIFO() calls that returned Result::Ok.
            size_t      readCount;              ///< Number of ReadFIFO() calls (manual or automatic) that drained at
                                                ///< least one capture.
        };

        constexpr Manager(SPI_HandleTypeDef * hspiCfg, GPIO_TypeDef * csCfgPort, uint16_t csCfgPin,
                           SPI_HandleTypeDef * hspiData, GPIO_TypeDef * csDataPort, uint16_t csDataPin,
                           GPIO_TypeDef * fifoFullPort, uint16_t fifoFullPin,
                           GPIO_TypeDef * fifoStartPort, uint16_t fifoStartPin) :
            config(hspiCfg, csCfgPort, csCfgPin),
            immediateFIFO(&config, hspiData, csDataPort, csDataPin, fifoFullPort, fifoFullPin),
            eventFIFO(&config, hspiData, csDataPort, csDataPin, fifoFullPort, fifoFullPin, fifoStartPort, fifoStartPin),
            triggerMode(TriggerMode::Immediate),
            readMode(ReadMode::Manual),
            captureInFlight(false),
            captureJustCompleted(false),
            inFlightOffsetSamples(0),
            inFlightWatermark(0),
            pendingCaptures{},
            pendingCaptureCount(0),
            pendingSampleCount(0),
            lastWatermark(DefaultWatermark),
            armCount(0),
            rearmCount(0),
            readCount(0),
            rxBuffer{}
        {
        };

        void Init();

        bool ScratchPadLoopback(uint8_t value);
        int  CheckDefaults();
        bool ChipInfo();

        Result SetTriggerMode(TriggerMode mode);
        Result SetReadMode(ReadMode mode);
        Result ArmFIFO(uint16_t watermark);
        Result RearmFIFO();
        Result ReadFIFO();
        FIFOInfo GetFIFOInfo() const;
        size_t AvailableBufferSpace() const;

        void OnFIFOFullISR();
        void OnDMACompleteISR();
        void PollFIFO();

#if defined(AD408X_MANAGER_SINGLETON)
        void RegisterAsSingleton();
        static const Manager & Instance();
#endif

    private:
        /// One completed, unread capture's location and size within rxBuffer.
        struct CaptureBoundary {
            size_t offsetSamples;   ///< Offset into rxBuffer, in samples (not bytes), where this capture starts.
            size_t sampleCount;     ///< Number of samples in this capture (its watermark at arm/rearm time).
        };

        /// @return A reference to whichever DataFIFO subclass triggerMode currently selects.
        DataFIFO & CurrentDataFIFO();

        /**
         * @brief Shared guard/bookkeeping sequence for ArmFIFO() and RearmFIFO().
         *
         * Checks captureInFlight, the watermark bound, and available buffer space (in that order, returning the first
         * applicable rejection), then claims the next free region of rxBuffer and calls CurrentDataFIFO().Arm() or
         * CurrentDataFIFO().Rearm() as appropriate.
         *
         * @param[in] watermark Watermark to arm/rearm with.
         * @param[in] isRearm   false to call CurrentDataFIFO().Arm() (and increment armCount) on success, true to call
         *                      CurrentDataFIFO().Rearm() (and increment rearmCount) on success instead.
         * @return Result::Ok on success, or the first rejection reason that applied.
         */
        Result ArmOrRearm(uint16_t watermark, bool isRearm);

        /**
         * @brief Logs each pending capture's boundary (offset/count/first/last sample) plus one aggregate summary line,
         *        then clears pendingCaptures/pendingCaptureCount/pendingSampleCount and increments readCount.
         *
         * Shared by ReadFIFO() (manual) and PollFIFO()'s automatic-read path. Caller is responsible for having already
         * checked pendingCaptureCount != 0.
         */
        void DrainPendingCaptures();

        Config               config;        ///< Config bus driver, shared by both FIFO objects below.
        ImmediateTriggerFIFO immediateFIFO; ///< Data bus driver for immediate trigger mode.
        EventTriggerFIFO     eventFIFO;     ///< Data bus driver for event trigger mode.

        TriggerMode triggerMode;    ///< Trigger mode ArmFIFO()/RearmFIFO() currently apply to.
        ReadMode    readMode;       ///< Whether PollFIFO() drains a completed capture automatically.

        volatile bool captureInFlight;      ///< True from a successful ArmFIFO()/RearmFIFO() until OnDMACompleteISR().
        volatile bool captureJustCompleted; ///< Set by OnDMACompleteISR(), consumed and cleared by PollFIFO().

        size_t   inFlightOffsetSamples; ///< Offset claimed for the capture currently in flight, if any.
        uint16_t inFlightWatermark;     ///< Watermark of the capture currently in flight, if any.

        CaptureBoundary pendingCaptures[MaxPendingCaptures];    ///< Completed, unread captures, oldest first.
        size_t          pendingCaptureCount;                    ///< Number of valid entries in pendingCaptures[].
        size_t          pendingSampleCount;                     ///< Total samples across all of pendingCaptures[].

        uint16_t lastWatermark; ///< Watermark of the most recent successful ArmFIFO(); seeds RearmFIFO().
        size_t   armCount;      ///< Total successful ArmFIFO() calls.
        size_t   rearmCount;    ///< Total successful RearmFIFO() calls.
        size_t   readCount;     ///< Total ReadFIFO() drains (manual or automatic).

        /// Backs every capture, both trigger modes; sized for the worst case of RxBufferMaxSamples samples' worth of
        /// pending, unread captures all outstanding at once.
        uint8_t rxBuffer[RxBufferMaxSamples * DataFIFO::BytesPerSample];

#if defined(AD408X_MANAGER_SINGLETON)
        static const Manager * instance; ///< Set once by RegisterAsSingleton(); read by Instance().
#endif
    };

}

/**
 * @file AD408xManager.cpp
 * @brief Implementation of the AD408x::Manager high-order test interface.
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "AD408xManager.hpp"

#include "DiagnosticsQ.h"
#include "sys_command_line.h"

#include <cassert>

// Swap this single line to redirect all PRINTF() calls in this file:
#define PRINTF(...)                     DiagQ_printf(__VA_ARGS__)
//#define PRINTF(...)                     DiagQ_Log(DiagSource_AD408xManager, ##__VA_ARGS__)

using namespace AD408x;

namespace {

    /**
     * @brief Interprets three sample bytes as a raw big-endian 24-bit value.
     *
     * @note AD4080 samples are 20-bit; the exact bit justification (left- vs right-justified within the 24-bit DATA
     * bus frame) has not been confirmed against the datasheet's ADC_DATA_INTF_CONFIG description yet, so this returns
     * the raw wire value, not a fully justified/sign-extended sample. Revisit once that's confirmed.
     *
     * @param[in] threeBytes Pointer to the first of 3 consecutive sample bytes (DataFIFO::BytesPerSample).
     * @return The 3 bytes packed MSB-first into the low 24 bits of a uint32_t.
     */
    uint32_t DecodeSampleRaw(uint8_t const *threeBytes) {
        return (static_cast<uint32_t>(threeBytes[0]) << 16) |
               (static_cast<uint32_t>(threeBytes[1]) <<  8) |
               (static_cast<uint32_t>(threeBytes[2]) <<  0);
    }

}

#if defined(AD408X_MANAGER_SINGLETON)
const Manager * Manager::instance = nullptr;
#endif

/* Internal functions ----------------------------------------------------------------------------*/

DataFIFO & Manager::CurrentDataFIFO() {
    return (triggerMode == TriggerMode::Event) ? (static_cast<DataFIFO &>(eventFIFO))
                                               : (static_cast<DataFIFO &>(immediateFIFO));
}

Manager::Result Manager::ArmOrRearm(uint16_t watermark, bool isRearm) {
    if (captureInFlight) {
        return Result::CaptureInFlight;
    }
    if ((watermark == 0) || (watermark > RxBufferMaxSamples)) {
        return Result::InvalidWatermark;
    }
    if (pendingCaptureCount >= MaxPendingCaptures) {
        return Result::MaxPendingCapturesReached;
    }
    if ((pendingSampleCount + watermark) > RxBufferMaxSamples) {
        return Result::InsufficientBufferSpace;
    }

    uint8_t * dest = &rxBuffer[pendingSampleCount * DataFIFO::BytesPerSample];
    inFlightOffsetSamples = pendingSampleCount;
    inFlightWatermark = watermark;
    captureInFlight = true;

    if (isRearm) {
        CurrentDataFIFO().Rearm(dest);
        ++rearmCount;
        PRINTF("%s: rearmed, watermark=%u (unchanged), offset=%u" ENDL,
            Name, (unsigned) watermark, (unsigned) inFlightOffsetSamples);
    } else {
        CurrentDataFIFO().Arm(watermark, dest);
        ++armCount;
        PRINTF("%s: armed, watermark=%u, offset=%u" ENDL,
            Name, (unsigned) watermark, (unsigned) inFlightOffsetSamples);
    }
    lastWatermark = watermark;

    if (triggerMode == TriggerMode::Event) {
        // Level triggered: assert FIFO_START now that FIFO_MODE is written. Deasserted in OnFIFOFullISR() once the
        // burst is confirmed to have started, per EventTriggerFIFO::TriggerEvent()'s doc comment.
        eventFIFO.TriggerEvent(true);
    }

    return Result::Ok;
}

void Manager::DrainPendingCaptures() {
    size_t totalSamples = 0;

    for (size_t index = 0; index < pendingCaptureCount; ++index) {
        CaptureBoundary const &capture = pendingCaptures[index];
        uint8_t const *first = &rxBuffer[capture.offsetSamples * DataFIFO::BytesPerSample];
        uint8_t const *last =
            &rxBuffer[(capture.offsetSamples + capture.sampleCount - 1) * DataFIFO::BytesPerSample];

        PRINTF("%s: capture %u/%u: %u samples @ offset %u, first=0x%06lx, last=0x%06lx" ENDL,
            Name,
            (unsigned) (index + 1), (unsigned) pendingCaptureCount, (unsigned) capture.sampleCount,
            (unsigned) capture.offsetSamples, (unsigned long) DecodeSampleRaw(first),
            (unsigned long) DecodeSampleRaw(last));

        totalSamples += capture.sampleCount;
    }

    PRINTF("%s: read complete, %u capture(s), %u total samples drained" ENDL,
        Name, (unsigned) pendingCaptureCount, (unsigned) totalSamples);

    pendingCaptureCount = 0;
    pendingSampleCount = 0;
    ++readCount;
}

/* External functions ----------------------------------------------------------------------------*/

void Manager::Init() {
    config.Init();
    immediateFIFO.Init();
    eventFIFO.Init();
}

bool Manager::ScratchPadLoopback(uint8_t value) {
    return config.ScratchPadLoopback(value);
}

int Manager::CheckDefaults() {
    int invalid = config.TestReadAll();
    PRINTF("%s: CheckDefaults() found %d register(s) not at reset value" ENDL, Name, invalid);
    return invalid;
}

bool Manager::ChipInfo() {
    return config.VerifyChipID();
}

Manager::Result Manager::SetTriggerMode(TriggerMode mode) {
    if (captureInFlight) {
        return Result::CaptureInFlight;
    }
    triggerMode = mode;
    PRINTF("%s: trigger mode set to %s" ENDL, Name, (mode == TriggerMode::Event) ? ("event") : ("immediate"));
    return Result::Ok;
}

Manager::Result Manager::SetReadMode(ReadMode mode) {
    // Not guarded by captureInFlight: this only changes whether PollFIFO() auto-drains a future completion, it
    // does not touch the AD408x or the in-flight capture itself.
    readMode = mode;
    PRINTF("%s: read mode set to %s" ENDL, Name, (mode == ReadMode::Automatic) ? ("automatic") : ("manual"));
    return Result::Ok;
}

Manager::Result Manager::ArmFIFO(uint16_t watermark) {
    return ArmOrRearm(watermark, false);
}

Manager::Result Manager::RearmFIFO() {
    return ArmOrRearm(lastWatermark, true);
}

Manager::Result Manager::ReadFIFO() {
    if (pendingCaptureCount == 0) {
        return Result::NothingPending;
    }
    DrainPendingCaptures();
    return Result::Ok;
}

Manager::FIFOInfo Manager::GetFIFOInfo() const {
    FIFOInfo info;
    info.triggerMode = triggerMode;
    info.readMode = readMode;
    info.captureInFlight = captureInFlight;
    info.pendingCaptureCount = pendingCaptureCount;
    info.pendingSampleCount = pendingSampleCount;
    info.lastWatermark = lastWatermark;
    info.armCount = armCount;
    info.rearmCount = rearmCount;
    info.readCount = readCount;
    return info;
}

size_t Manager::AvailableBufferSpace() const {
    return RxBufferMaxSamples - pendingSampleCount;
}

void Manager::OnFIFOFullISR() {
    CurrentDataFIFO().OnFIFOFullISR();

    if (triggerMode == TriggerMode::Event) {
        // Burst is now started (DMA kicked off above); deassert FIFO_START so it isn't left high across a later
        // Rearm(), per EventTriggerFIFO::TriggerEvent()'s doc comment.
        eventFIFO.TriggerEvent(false);
    }
}

void Manager::OnDMACompleteISR() {
    CurrentDataFIFO().OnDMACompleteISR();

    captureInFlight = false;

    if (pendingCaptureCount < MaxPendingCaptures) {
        pendingCaptures[pendingCaptureCount].offsetSamples = inFlightOffsetSamples;
        pendingCaptures[pendingCaptureCount].sampleCount = inFlightWatermark;
        ++pendingCaptureCount;
    }
    pendingSampleCount += inFlightWatermark;

    captureJustCompleted = true;
}

void Manager::PollFIFO() {
    if (!captureJustCompleted) {
        return;
    }
    captureJustCompleted = false;

    if (readMode == ReadMode::Automatic) {
        DrainPendingCaptures();
    }
}

#if defined(AD408X_MANAGER_SINGLETON)

void Manager::RegisterAsSingleton() {
    assert(instance == nullptr); // register exactly once
    instance = this;
}

const Manager & Manager::Instance() {
    assert(instance != nullptr); // RegisterAsSingleton() must have run first
    return *instance;
}

#endif

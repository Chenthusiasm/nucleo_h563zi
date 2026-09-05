/** @file       ADCCmd.cpp
 *  @brief      CLI "adc" command implementation.
 *
 * Defines the argument parsing/validation for the "adc" command and its subcommands. This module
 * does not touch the AD408x::Config/DataFIFO instances directly, those are owned by MainAppTask,
 * since they own the CFG/DATA bus SPI peripherals and must serialize access against a possibly
 * pending FIFO capture. Each subcommand here simply validates its arguments and enqueues a request
 * into MainAppQ; MainAppTask is responsible for actually calling into AD408x::Config and for
 * rejecting a request if the bus is busy.
 *
 * Because the ADC access itself happens asynchronously in MainAppTask, and AD408x::Config's
 * ScratchPadLoopback()/VerifyChipID()/TestReadAll() already LOG() their results internally (which
 * routes through DiagQ_printf/DiagQ_Log), this command's synchronous EXIT_SUCCESS/EXIT_FAILURE only
 * reflects whether the request was successfully queued, not the outcome of the ADC access itself.
 * The actual result shows up a moment later in the diagnostics log. Same story for the "fifo"
 * subcommands: MainAppTask owns the AD408x::Manager instance and all of its state; this file only
 * validates arguments and enqueues.
 *
 * The "fifo arm" subcommand is the one exception with a synchronous check: this file reads
 * AD408x::Manager::Instance() (a read-only singleton accessor; see AD408xManager.hpp) to reject an
 * obviously oversized watermark immediately, with the actual available sample count in the error
 * message, rather than waiting on a queued rejection from MainAppTask. That singleton access is
 * read-only by construction (Instance() returns a const Manager&, and every mutating Manager method
 * is non-const), so this file cannot accidentally bypass MainAppQ to mutate ADC/FIFO state
 * directly.
 *
 * The CLI handling of the ADC commands happens on the DiagnosticsTask, which periodically calls
 * CLI_RUN().
 */

/* Includes ----------------------------------------------------------------------------------- */

#include "ADCCmd.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "sys_command_line.h"
#include "MainAppQ.h"
#include "CmdHelper.h"
#include "AD408xManager.hpp"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define CMD_NAME                        "adc"

#define SUBCMD_HELP                     "help"
#define SUBCMD_SCRATCHPAD               "scratchpad"
#define SUBCMD_CHECKDEFAULTS            "checkdefaults"
#define SUBCMD_INFO                     "info"
#define SUBCMD_FIFO                     "fifo"

#define FIFO_PARAM_INFO                 "info"
#define FIFO_PARAM_IMMEDIATE            "immediate"
#define FIFO_PARAM_EVENT                "event"
#define FIFO_PARAM_AUTOREAD             "autoread"
#define FIFO_PARAM_MANUALREAD           "manualread"
#define FIFO_PARAM_ARM                  "arm"
#define FIFO_PARAM_REARM                "rearm"
#define FIFO_PARAM_READ                 "read"


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static char const CmdHelp[] =
    "Controls the AD408x ADC." ENDL
    "\tadc help                   - print this usage text" ENDL
    "\tadc scratchpad <value>     - write <value> (decimal or 0x.. hex) to SCRATCH_PAD and read it back to confirm it "
    "matches" ENDL
    "\tadc checkdefaults          - read all config registers and check them against reset defaults" ENDL
    "\tadc info                   - read and print CHIP_TYPE/PRODUCT_ID/CHIP_GRADE" ENDL
    "\tadc fifo [info]            - print FIFO mode, read mode, watermark, arm/rearm/read counts" ENDL
    "\tadc fifo immediate         - select ImmediateTriggerFIFO for subsequent arm/rearm" ENDL
    "\tadc fifo event             - select EventTriggerFIFO for subsequent arm/rearm" ENDL
    "\tadc fifo autoread          - drain and log a capture automatically as soon as it completes" ENDL
    "\tadc fifo manualread        - leave completed captures pending until \"adc fifo read\"" ENDL
    "\tadc fifo arm <watermark>   - arm with <watermark> samples" ENDL
    "\tadc fifo rearm             - rearm with the last watermark (default if never armed)" ENDL
    "\tadc fifo read              - manually drain any pending, unread captures" ENDL
    "Note: subcommands are queued to MainAppTask; the actual pass/fail result is printed asynchronously in the log "
    "once MainAppTask executes the request against the ADC.";


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void printCmdUsage() {
    printf("Usage: " CMD_NAME " <"
        SUBCMD_HELP "|"
        SUBCMD_SCRATCHPAD "|"
        SUBCMD_CHECKDEFAULTS "|"
        SUBCMD_INFO "|"
        SUBCMD_FIFO
        "> [args]" ENDL);
}

static void printSubcmdUsage(char const* subcmd, char const* argsString = nullptr) {
    if (!argsString) {
        printf("Usage: " CMD_NAME " %s" ENDL, subcmd);
    } else {
        printf("Usage: " CMD_NAME " %s %s" ENDL, subcmd, argsString);
    }
}

static void printEnqueueFailure(char const* subcmd) {
    printf("Could not queue %s request (MainAppTask busy or queue full)." ENDL, subcmd);
}

static uint8_t handleSubcmd_help(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    printf("%s" ENDL, CmdHelp);
    return EXIT_SUCCESS;
}

static uint8_t handleSubcmd_scratchpad(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_SCRATCHPAD, "<value>");
        return EXIT_FAILURE;
    }

    uint8_t value;
    if (!ParseStringUInt8(argv[2], &value)) {
        printf("\"%s\" is not a valid byte value (0-255, or 0x00-0xff)." ENDL, argv[2]);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdScratchPad(value)) {
        printEnqueueFailure(SUBCMD_SCRATCHPAD);
        return EXIT_FAILURE;
    }

    printf("Queued SCRATCH_PAD loopback with 0x%02x; watch the log for the result." ENDL, value);
    return EXIT_SUCCESS;
}

static uint8_t handleSubcmd_checkdefaults(int argc, char *argv[]) {
    if (argc != 2) {
        printSubcmdUsage(SUBCMD_CHECKDEFAULTS);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdCheckDefaults()) {
        printEnqueueFailure(SUBCMD_CHECKDEFAULTS);
        return EXIT_FAILURE;
    }

    printf("Queued all config register check against defaults; watch the log for any mismatches." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleSubcmd_info(int argc, char *argv[]) {
    if (argc != 2) {
        printSubcmdUsage(SUBCMD_INFO);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdInfo()) {
        printEnqueueFailure(SUBCMD_INFO);
        return EXIT_FAILURE;
    }

    printf("Queued chip ID readback; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_info(int argc, char *argv[]) {
    if ((argc != 2) && (argc != 3)) {
        printSubcmdUsage(SUBCMD_FIFO, "[" FIFO_PARAM_INFO "]");
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOInfo()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_INFO);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO info request; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_immediate(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_IMMEDIATE);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOImmediate()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_IMMEDIATE);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO mode switch to immediate; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_event(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_EVENT);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOEvent()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_EVENT);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO mode switch to event; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_autoread(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_AUTOREAD);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOAutoRead()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_AUTOREAD);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO read mode switch to automatic; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_manualread(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_MANUALREAD);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOManualRead()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_MANUALREAD);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO read mode switch to manual; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_arm(int argc, char *argv[]) {
    if (argc != 4) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_ARM " <watermark>");
        return EXIT_FAILURE;
    }

    uint16_t watermark;
    if (!ParseStringUInt16(argv[3], &watermark)) {
        printf("\"%s\" is not a valid watermark value." ENDL, argv[3]);
        return EXIT_FAILURE;
    }
    if (watermark == 0) {
        printf("Watermark must be at least 1." ENDL);
        return EXIT_FAILURE;
    }
    if (watermark > AD408x::Manager::RxBufferMaxSamples) {
        printf("Watermark must not exceed %u." ENDL, (unsigned) AD408x::Manager::RxBufferMaxSamples);
        return EXIT_FAILURE;
    }

    // Synchronous read-only check against the live singleton: rejects an oversized request immediately, with the
    // actual available count, instead of waiting on a queued rejection from MainAppTask. See the file-level
    // comment above for why this is safe (Instance() returns a const Manager&).
    std::size_t available = AD408x::Manager::Instance().AvailableBufferSpace();
    if (watermark > available) {
        printf("Watermark %u exceeds the %u sample(s) currently available; read pending captures first." ENDL,
               (unsigned) watermark, (unsigned) available);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFOArm(watermark)) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_ARM);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO arm with watermark %u; watch the log for the result." ENDL, (unsigned) watermark);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_rearm(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_REARM);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFORearm()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_REARM);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO rearm; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleFifo_read(int argc, char *argv[]) {
    if (argc != 3) {
        printSubcmdUsage(SUBCMD_FIFO, FIFO_PARAM_READ);
        return EXIT_FAILURE;
    }

    if (!MainAppQ_ADCCmdFIFORead()) {
        printEnqueueFailure(SUBCMD_FIFO " " FIFO_PARAM_READ);
        return EXIT_FAILURE;
    }

    printf("Queued FIFO read; watch the log for the result." ENDL);
    return EXIT_SUCCESS;
}

static uint8_t handleSubcmd_fifo(int argc, char *argv[]) {
    if (argc == 2) {
        return handleFifo_info(argc, argv);
    }

    char const* param = argv[2];
    if (strcmp(param, FIFO_PARAM_INFO) == 0) {
        return handleFifo_info(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_IMMEDIATE) == 0) {
        return handleFifo_immediate(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_EVENT) == 0) {
        return handleFifo_event(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_AUTOREAD) == 0) {
        return handleFifo_autoread(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_MANUALREAD) == 0) {
        return handleFifo_manualread(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_ARM) == 0) {
        return handleFifo_arm(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_REARM) == 0) {
        return handleFifo_rearm(argc, argv);
    } else if (strcmp(param, FIFO_PARAM_READ) == 0) {
        return handleFifo_read(argc, argv);
    }

    printf("Unknown \"" CMD_NAME " " SUBCMD_FIFO "\" parameter \"%s\". Try \"" CMD_NAME " " SUBCMD_HELP "\"." ENDL,
           param);
    return EXIT_FAILURE;
}

static uint8_t handleCmd(int argc, char *argv[]) {
    if (argc < 2) {
        printCmdUsage();
        return EXIT_FAILURE;
    }
    char const* subcmd = argv[1];
    if (strcmp(subcmd, SUBCMD_HELP) == 0) {
        return handleSubcmd_help(argc, argv);
    } else if (strcmp(subcmd, SUBCMD_SCRATCHPAD) == 0) {
        return handleSubcmd_scratchpad(argc, argv);
    } else if (strcmp(subcmd, SUBCMD_CHECKDEFAULTS) == 0) {
        return handleSubcmd_checkdefaults(argc, argv);
    } else if (strcmp(subcmd, SUBCMD_INFO) == 0) {
        return handleSubcmd_info(argc, argv);
    } else if (strcmp(subcmd, SUBCMD_FIFO) == 0) {
        return handleSubcmd_fifo(argc, argv);
    }

    printf("Unknown \"" CMD_NAME "\" subcommand \"%s\". Try \"" CMD_NAME " " SUBCMD_HELP "\"." ENDL, subcmd);
    return EXIT_FAILURE;
}


/* External define -------------------------------------------------------------------------------*/

char const* ADCCmdSubcmdNames[] = {
    // this array initializer is not standard in C++17, it's part of the C99/GNU extension; possibly not portable
    [ADCCmdSubcmd_scratchpad]    = SUBCMD_SCRATCHPAD,
    [ADCCmdSubcmd_checkdefaults] = SUBCMD_CHECKDEFAULTS,
    [ADCCmdSubcmd_info]          = SUBCMD_INFO,
    [ADCCmdSubcmd_fifo]          = SUBCMD_FIFO,
};

char const* ADCCmdFIFOParameterNames[] = {
    // this array initializer is not standard in C++17, it's part of the C99/GNU extension; possibly not portable
    [ADCCmdFIFOParameter_info]       = FIFO_PARAM_INFO,
    [ADCCmdFIFOParameter_immediate]  = FIFO_PARAM_IMMEDIATE,
    [ADCCmdFIFOParameter_event]      = FIFO_PARAM_EVENT,
    [ADCCmdFIFOParameter_autoread]   = FIFO_PARAM_AUTOREAD,
    [ADCCmdFIFOParameter_manualread] = FIFO_PARAM_MANUALREAD,
    [ADCCmdFIFOParameter_arm]        = FIFO_PARAM_ARM,
    [ADCCmdFIFOParameter_rearm]      = FIFO_PARAM_REARM,
    [ADCCmdFIFOParameter_read]       = FIFO_PARAM_READ,
};

/* External functions ----------------------------------------------------------------------------*/

void ADCCmd_Init(void) {
    CLI_ADD_CMD(CMD_NAME, CmdHelp, handleCmd);
}

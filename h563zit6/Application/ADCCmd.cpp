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
 * reflects whether the request was successfully queued -- not the outcome of the ADC access itself.
 * The actual result shows up a moment later in the diagnostics log.
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


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/

#define CMD_NAME                        "adc"
#define SUBCMD_HELP                     "help"
#define SUBCMD_SCRATCHPAD               "scratchpad"
#define SUBCMD_CHECKDEFAULTS            "checkdefaults"
#define SUBCMD_INFO                     "info"


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/

static char const CmdHelp[] =
    "Controls the AD408x ADC." ENDL
    "\tadc help                - print this usage text" ENDL
    "\tadc scratchpad <value>  - write <value> (decimal or 0x.. hex) to SCRATCH_PAD and read it" ENDL
    "\t                          back to confirm it matches" ENDL
    "\tadc checkdefaults       - read all config registers and check them against reset defaults" ENDL
    "\tadc info                - read and print CHIP_TYPE/PRODUCT_ID/CHIP_GRADE" ENDL
    "Note: subcommands are queued to MainAppTask; the actual pass/fail result is printed" ENDL
    "asynchronously in the log once MainAppTask executes the request against the ADC.";


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static void printCmdUsage() {
    printf("Usage: " CMD_NAME " <"
        SUBCMD_HELP "|"
        SUBCMD_SCRATCHPAD "|"
        SUBCMD_CHECKDEFAULTS "|"
        SUBCMD_INFO
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
    }

    printf("Unknown \"" CMD_NAME "\" subcommand \"%s\". Try \"" CMD_NAME " " SUBCMD_HELP "\"." ENDL, subcmd);
    return EXIT_FAILURE;
}


/* External functions ----------------------------------------------------------------------------*/

void ADCCmd_Init(void) {
    CLI_ADD_CMD(CMD_NAME, CmdHelp, handleCmd);
}

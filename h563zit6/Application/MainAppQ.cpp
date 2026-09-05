/* Includes --------------------------------------------------------------------------------------*/

#include "MainAppQ.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "ADCCmdDefines.h"
#include "app_freertos.h"
#include "RTOS.h"


/* Internal typedef ------------------------------------------------------------------------------*/


/* Internal define -------------------------------------------------------------------------------*/


/* Internal macro --------------------------------------------------------------------------------*/


/* Internal variables ----------------------------------------------------------------------------*/


/* Internal function prototypes ------------------------------------------------------------------*/


/* Internal functions ----------------------------------------------------------------------------*/

static bool enqueueEvent(MainAppEvent_t event, MainAppMsgContent_t * content) {
    if (!content) {
        return false;
    }
    MainAppMsg_t message = {
        .event = event,
        .content = *content,
    };
    osStatus_t status = osMessageQueuePut(MainAppQHandle, &message, 0u, 0u);
    return (status == osOK);
}


/* External functions ----------------------------------------------------------------------------*/

bool MainAppQ_Sleep_ms(uint32_t duration_ms) {
    MainAppMsgContent_t content = {
        .sleepTime_ms = duration_ms,
    };
    return enqueueEvent(MainAppEvent_Sleep, &content);
}


bool MainAppQ_UserButtonPressed(void) {
    MainAppMsgContent_t content = {
        .buttonTransition = ButtonTransition_Pressed,
    };
    return enqueueEvent(MainAppEvent_UserButtonChange, &content);
}


bool MainAppQ_UserButtonReleased(void) {
    MainAppMsgContent_t content = {
        .buttonTransition = ButtonTransition_Released,
    };
    return enqueueEvent(MainAppEvent_UserButtonChange, &content);
}

bool MainAppQ_ADCCmdScratchPad(uint8_t scratchpadValue) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_scratchpad,
            .values = {
                .scratchpadValue = scratchpadValue,
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdCheckDefaults(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_checkdefaults,
            //values doesn't matter
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdInfo(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_info,
            //values doesn't matter
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOInfo(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_info,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOImmediate(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_immediate,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOEvent(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_event,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOAutoRead(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_autoread,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOManualRead(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_manualread,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFOArm(uint16_t watermark) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_arm,
                    .watermark = watermark,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFORearm(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_rearm,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

bool MainAppQ_ADCCmdFIFORead(void) {
    MainAppMsgContent_t content = {
        .adcCmd = {
            .subcmd = ADCCmdSubcmd_fifo,
            .values = {
                .fifo = {
                    .parameter = ADCCmdFIFOParameter_read,
                }
            }
        }
    };
    return enqueueEvent(MainAppEvent_ADCCmd, &content);
}

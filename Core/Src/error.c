#include "error.h"
#include "main.h"

void error_set(uint8_t error)
{
    SEGGER_RTT_printf(0, "error:%02x\n", error);
}

#include "main.h"


#include "string.h"
/* RTOS ----------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

/* User Task -----------------------------------------------------------------*/
#include "schedule_main.h"
#include "dxl_main.h"

/* Component -----------------------------------------------------------------*/
#include "UART_Class.h"
#include "DXL_Class.h"
#include "DXL_Manager.h"
#include "cpp_tick.h"

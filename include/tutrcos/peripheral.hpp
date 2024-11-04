#pragma once

#include "main.h"

#ifdef HAL_GPIO_MODULE_ENABLED
#include "peripheral/gpio.hpp"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
#include "peripheral/encoder.hpp"
#include "peripheral/pwm.hpp"
#endif

#ifdef HAL_UART_MODULE_ENABLED
#include "peripheral/uart.hpp"
#endif

#ifdef HAL_CAN_MODULE_ENABLED
#include "peripheral/can.hpp"
#endif

#ifdef HAL_FDCAN_MODULE_ENABLED
#include "coperipheralre/fdcan.hpp"
#endif

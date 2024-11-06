/*
 * hal_uart.h
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef SRC_HAL_INCLUDE_HAL_UART_H_
#define SRC_HAL_INCLUDE_HAL_UART_H_

#include <stdint.h>
#include <msp430.h>

#include "hal_gpio.h"

#define UART_PORT               (GPIO_PORT_1)
#define UART_PIN_TX             (0x04u)

#define UART_SEND_MAX_LEN       ((uint8_t) 16u)
#define UART_BITRATE_DIVIDER    ((uint16_t) 69u)

void hal_uart_init(void);

uint8_t hal_uart_send(const uint8_t * const buf, uint8_t len);

#endif /* SRC_HAL_INCLUDE_HAL_UART_H_ */

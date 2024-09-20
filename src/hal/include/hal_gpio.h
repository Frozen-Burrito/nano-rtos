/*
 * hal_gpio.h
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef SRC_HAL_INCLUDE_HAL_GPIO_H_
#define SRC_HAL_INCLUDE_HAL_GPIO_H_

#include <stdint.h>
#include <msp430.h>

typedef uint8_t gpio_port_t;

#define GPIO_PORT_1 ((gpio_port_t) 0U)
#define GPIO_PORT_2 ((gpio_port_t) 1U)

#define PIN_ALT_0   ((uint8_t) 0x00u)
#define PIN_ALT_1   ((uint8_t) 0x02u)
#define PIN_ALT_2   ((uint8_t) 0x04u)
#define PIN_ALT_3   ((uint8_t) 0x06u)

#define GPIO_DIRECTION_OUTPUT   ((uint8_t) 0x01u)
#define GPIO_INTERRUPT_FALLEDGE ((uint8_t) 0x08u)

void hal_gpio_init(gpio_port_t port, uint8_t mask, uint8_t config);

void hal_gpio_set(gpio_port_t port, uint8_t mask);
void hal_gpio_reset(gpio_port_t port, uint8_t mask);
void hal_gpio_toggle(gpio_port_t port, uint8_t mask);

#endif /* SRC_HAL_INCLUDE_HAL_GPIO_H_ */

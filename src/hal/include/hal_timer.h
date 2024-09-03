/*
 * hal_timer.h
 *
 *  Created on: Aug 31, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef SRC_HAL_INCLUDE_HAL_TIMER_H_
#define SRC_HAL_INCLUDE_HAL_TIMER_H_

#include <stdint.h>

#include <msp430.h>

#define HAL_TIMER_INIT() ({ \
    TA0CTL |= TASSEL_2 | MC_2 | TACLR; \
})

void hal_timer_delay(uint16_t milliseconds);

#endif /* SRC_HAL_INCLUDE_HAL_TIMER_H_ */

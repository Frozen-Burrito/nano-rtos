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

#define HAL_TIMER_INIT(timer) ({\
    if (0u == timer)\
    {\
        TA0CTL |= TASSEL_2 | MC_2 | TACLR;\
    }\
    else if (1u == timer)\
    {\
        TA1CTL |= TASSEL_2 | MC_2 | TACLR;\
    }\
})

#define HAL_TIMER_A0_0_START(count) ({\
    TA0CCR0 += count;\
    TA0CCTL0 |= CCIE;\
})

void hal_timer_delay(uint16_t milliseconds);

#endif /* SRC_HAL_INCLUDE_HAL_TIMER_H_ */

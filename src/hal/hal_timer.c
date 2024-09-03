/*
 * hal_timer.c
 *
 *  Created on: Aug 31, 2024
 *      Author: Fernando Mendoza V.
 */
#include "hal_timer.h"

void hal_timer_delay(uint16_t milliseconds)
{
    uint16_t count = milliseconds;

    while (0u != count)
    {
        TA0CCR0 = TA0R + 1000u;
        TA0CCTL0 &= ~CCIFG;
        while (0u == (TA0CCTL0 & CCIFG));
        count--;
    }
}

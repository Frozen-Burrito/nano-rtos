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
        TA0CCR1 = TA0R + 8000u;
        TA0CCTL1 &= ~CCIFG;
        while (0u == (TA0CCTL1 & CCIFG));
        count--;
    }
}

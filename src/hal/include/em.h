/*
 * em.h
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef EM_H_
#define EM_H_

#include <msp430.h>

#define EM_GLOBAL_INTERRUPT_EN  (__bis_SR_register(GIE))
#define EM_GLOBAL_INTERRUPT_DIS (__bic_SR_register(GIE))

#define EM_SLEEP_ENTER          (__bis_SR_register(GIE | LPM0_bits))

#define EM_SLEEP_EXIT           (__bic_SR_register(LPM4_bits))

#define WATCHDOG_STOP           (WDTCTL = WDTPW | WDTHOLD)

#define DCO_CAL_1MHZ() ({\
    DCOCTL = 0;\
    BCSCTL1 = CALBC1_1MHZ;\
    DCOCTL = CALDCO_1MHZ;\
})

#define DCO_CAL_8MHZ() ({\
    DCOCTL = 0;\
    BCSCTL1 = CALBC1_8MHZ;\
    DCOCTL = CALDCO_8MHZ;\
})

#endif /* EM_H_ */

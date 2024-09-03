/*
 * hal_uart.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */
#include "hal_uart.h"

void hal_uart_init(void)
{
    UCA0CTL1 |= UCSWRST;

    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = (uint8_t) (UART_BITRATE_DIVIDER & 0xFFu);
    UCA0BR1 = (uint8_t) (UART_BITRATE_DIVIDER >> 8u);

    hal_gpio_init(UART_PORT, UART_PIN_TX, PIN_ALT_3);

    UCA0CTL1 &= ~UCSWRST;
}

void hal_uart_send(const uint8_t * const buf)
{
    uint8_t i = 0u;

    while ('\0' != buf[i] && UART_SEND_MAX_LEN > i)
    {
        while (0u == (IFG2 & UCA0TXIFG));
        UCA0TXBUF = buf[i++];
    }
}

/*
 * hal_uart.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */
#include "hal_uart.h"

static uint8_t transmit_buffer[UART_SEND_MAX_LEN];
static uint8_t transmit_len_bytes;
static uint8_t bytes_transmitted;

void hal_uart_init(void)
{
    UCA0CTL1 |= UCSWRST;

    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = (uint8_t) (UART_BITRATE_DIVIDER & 0xFFu);
    UCA0BR1 = (uint8_t) (UART_BITRATE_DIVIDER >> 8u);

    hal_gpio_init(UART_PORT, UART_PIN_TX, PIN_ALT_3);

    transmit_len_bytes = 0u;

    UCA0CTL1 &= ~UCSWRST;
}

void hal_uart_send(const uint8_t * const buf)
{
    if (0u == transmit_len_bytes && 0u != buf)
    {
        while ('\0' != buf[transmit_len_bytes] && UART_SEND_MAX_LEN > transmit_len_bytes)
        {
            transmit_buffer[transmit_len_bytes] = buf[transmit_len_bytes];
            transmit_len_bytes++;
        }

        bytes_transmitted = 0u;
        IE2 |= UCA0TXIE;
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void usciab_tx_isr(void)
{
    UCA0TXBUF = transmit_buffer[bytes_transmitted++];

    if (bytes_transmitted == transmit_len_bytes)
    {
        transmit_len_bytes = 0u;
        bytes_transmitted = 0u;
        IE2 &= ~UCA0TXIE;
    }
}

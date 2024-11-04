/*
 * hal_uart.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */
#include "hal_uart.h"

static volatile uint8_t transmit_buffer[UART_SEND_MAX_LEN];
static volatile uint8_t transmit_cursor;
static volatile uint8_t transmit_end;

void hal_uart_init(void)
{
    UCA0CTL1 |= UCSWRST;

    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = (uint8_t) (UART_BITRATE_DIVIDER & 0xFFu);
    UCA0BR1 = (uint8_t) (UART_BITRATE_DIVIDER >> 8u);

    hal_gpio_init(UART_PORT, UART_PIN_TX, PIN_ALT_3);

    transmit_cursor = UART_SEND_MAX_LEN - 1u;
    transmit_end = 0u;

    UCA0CTL1 &= ~UCSWRST;
}

uint8_t hal_uart_send(const uint8_t * const buf, uint8_t len)
{
    uint8_t i;

    i = 0u;
    if (UART_SEND_MAX_LEN > len && 0u != buf)
    {
        while (i < len && transmit_end != transmit_cursor)
        {
            transmit_buffer[transmit_end] = buf[i++];

            transmit_end++;
            if (UART_SEND_MAX_LEN <= transmit_end)
            {
                transmit_end = 0u;
            }
        }
    }

    if (0u != i)
    {
        IE2 |= UCA0TXIE;
    }

    return i;
}

//    if (0u == transmit_len_bytes && 0u != buf)
//    {
//        while ('\0' != buf[transmit_len_bytes] && UART_SEND_MAX_LEN > transmit_len_bytes)
//        {
//            transmit_buffer[transmit_len_bytes] = buf[transmit_len_bytes];
//            transmit_len_bytes++;
//        }
//
//        bytes_transmitted = 0u;
//        IE2 |= UCA0TXIE;
//    }

#pragma vector=USCIAB0TX_VECTOR
__interrupt void usciab_tx_isr(void)
{
    uint8_t i;

    transmit_cursor++;
    if (UART_SEND_MAX_LEN <= transmit_cursor)
    {
        transmit_cursor = 0u;
    }

    UCA0TXBUF = transmit_buffer[transmit_cursor];

    i = transmit_cursor + 1u;
    if (UART_SEND_MAX_LEN <= transmit_cursor)
    {
        i = 0u;
    }

    if (transmit_end == i)
    {
        transmit_cursor = UART_SEND_MAX_LEN - 1u;
        transmit_end = 0u;
        IE2 &= ~UCA0TXIE;
    }
}

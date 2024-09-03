/*
 * hal_gpio.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Fernando Mendoza V.
 */
#include "hal_gpio.h"

#define CONFIG_ALT_1_BIT    ((uint8_t) 0x02u)
#define CONFIG_ALT_2_BIT    ((uint8_t) 0x04u)

void hal_gpio_init(gpio_port_t port, uint8_t mask, uint8_t config)
{
    if (GPIO_PORT_1 == port)
    {
        if (0u != (GPIO_DIRECTION_OUTPUT & config))
        {
            P1OUT &= ~mask;
            P1DIR |= mask;
        }
        else
        {
            P1DIR &= ~mask;
        }

        if (0u != (CONFIG_ALT_1_BIT & config))
        {
            P1SEL |= mask;
        }
        else
        {
            P1SEL &= ~mask;
        }

        if (0u != (CONFIG_ALT_2_BIT & config))
        {
            P1SEL2 |= mask;
        }
        else
        {
            P1SEL2 &= ~mask;
        }
    }
    else if (GPIO_PORT_2 == port)
    {

    }
}

void hal_gpio_set(gpio_port_t port, uint8_t mask)
{
    if (GPIO_PORT_1 == port)
    {
        P1OUT |= mask;
    }
    else if (GPIO_PORT_2 == port)
    {
        P2OUT |= mask;
    }
}

void hal_gpio_reset(gpio_port_t port, uint8_t mask)
{
    if (GPIO_PORT_1 == port)
    {
        P1OUT &= ~mask;
    }
    else if (GPIO_PORT_2 == port)
    {
        P2OUT &= ~mask;
    }
}

void hal_gpio_toggle(gpio_port_t port, uint8_t mask)
{
    if (GPIO_PORT_1 == port)
    {
        P1OUT ^= mask;
    }
    else if (GPIO_PORT_2 == port)
    {
        P2OUT ^= mask;
    }
}

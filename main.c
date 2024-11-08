/*
 * main.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */
#include <stdint.h>

#include <msp430.h>

#include "em.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_uart.h"

#include "os/os.h"
#include "os/tasks.h"
#include "os/alarms.h"
#include "os/queues.h"

#define FALSE       ((uint8_t) 0u)
#define TRUE        ((uint8_t) 1u)

/* GPIO - UART - Puerto 1 */
#define UART_A_RX_PIN       ((uint8_t) 0x02u)
#define UART_B_RX_PIN       ((uint8_t) 0x08u)
#define UART_C_RX_PIN       ((uint8_t) 0x10u)

#define UART_A_PACKET_ID    ((uint8_t) 0u)
#define UART_B_PACKET_ID    ((uint8_t) 1u)
#define UART_C_PACKET_ID    ((uint8_t) 2u)

#define UART_BIT_COUNT      ((uint8_t) 8u)
#define UART_BIT_TIME_TICKS ((uint8_t) 4u)

/* GPIO - Puerto 2 */
#define PWM_R_PIN           ((uint8_t) 0x02u)
#define PWM_G_PIN           ((uint8_t) 0x08u)
#define PWM_B_PIN           ((uint8_t) 0x20u)

// 10k ticks por segundo?
#define PWM_PERIOD_TICKS    ((tick_type_t) 24u)

/* OS tasks */
#define PWM_PERIOD_TASK_ID  ((uint8_t) 0u)
#define PWM_R_TASK_ID       ((uint8_t) 1u)
#define PWM_G_TASK_ID       ((uint8_t) 2u)
#define PWM_B_TASK_ID       ((uint8_t) 3u)
#define UART_A_RX_TASK_ID   ((uint8_t) 4u)
#define UART_B_RX_TASK_ID   ((uint8_t) 5u)
#define UART_C_RX_TASK_ID   ((uint8_t) 6u)

#define PWM_PACKET_QUEUE_ID             ((queue_id_t) 0u)
#define PWM_PACKET_QUEUE_LENGTH         ((uint8_t) 4u)

typedef struct _pwm_packet_t {
    uint8_t id;
    uint8_t duty_cycle;
} pwm_packet_t;

static void pwm_r_task(void);
static void pwm_g_task(void);
static void pwm_b_task(void);
static void pwm_period_task(void);
static void uart_a_rx_task(void);
static void uart_b_rx_task(void);
static void uart_c_rx_task(void);

static uint8_t uart_a_rx_bits_pending;
static uint8_t uart_b_rx_bits_pending;
static uint8_t uart_c_rx_bits_pending;

int main(void)
{
	WATCHDOG_STOP;
	DCO_CAL_8MHZ();

	// Peripherals init.
    HAL_TIMER_INIT(0);

    hal_gpio_init(GPIO_PORT_1, (UART_A_RX_PIN | UART_B_RX_PIN | UART_C_RX_PIN), GPIO_INTERRUPT_FALLEDGE);

    hal_gpio_init(GPIO_PORT_2, (PWM_R_PIN | PWM_G_PIN | PWM_B_PIN), GPIO_DIRECTION_OUTPUT);
    hal_gpio_reset(GPIO_PORT_2, (PWM_R_PIN | PWM_G_PIN | PWM_B_PIN));

    EM_GLOBAL_INTERRUPT_EN;

    // OS init.
	os_task_create(PWM_PERIOD_TASK_ID, pwm_period_task, 2u, TRUE);
	os_task_create(PWM_R_TASK_ID, pwm_r_task, 1u, FALSE);
	os_task_create(PWM_G_TASK_ID, pwm_g_task, 1u, FALSE);
	os_task_create(PWM_B_TASK_ID, pwm_b_task, 1u, FALSE);

	os_task_create(UART_A_RX_TASK_ID, uart_a_rx_task, 3u, FALSE);
	os_task_create(UART_B_RX_TASK_ID, uart_b_rx_task, 3u, FALSE);
	os_task_create(UART_C_RX_TASK_ID, uart_c_rx_task, 3u, FALSE);

	os_queue_init(PWM_PACKET_QUEUE_ID, PWM_PACKET_QUEUE_LENGTH, (1u << PWM_PERIOD_TASK_ID) | (1u << UART_A_RX_TASK_ID) | (1u << UART_B_RX_TASK_ID) | (1u << UART_C_RX_TASK_ID));

	os_init();

	os_alarm_set_rel(ALARM_A, PWM_PERIOD_TICKS, PWM_PERIOD_TASK_ID, TRUE);

	scheduler_run();

	while(1);

	return 0;
}

void uart_a_rx_task(void)
{
    static volatile uint8_t rx_buf;
    static volatile pwm_packet_t pwm_packet;

    rx_buf |= (P1IN & UART_A_RX_PIN) << 6u;
    rx_buf >>= 1;

    if (UART_BIT_COUNT == uart_a_rx_bits_pending)
    {
        os_alarm_set_rel(ALARM_E, UART_BIT_TIME_TICKS, UART_A_RX_TASK_ID, TRUE);
    }

    if (0u == --uart_a_rx_bits_pending)
    {
        os_alarm_cancel(ALARM_E);

        if (PWM_PERIOD_TICKS >= rx_buf)
        {
            // Dato completo, todos los bits recibidos y stop bit es 1.
            pwm_packet.id = UART_A_PACKET_ID;
            pwm_packet.duty_cycle = rx_buf;

            // Enviar paquete con nuevo pwm a la queue.
            os_queue_send(PWM_PACKET_QUEUE_ID, (void *) &pwm_packet, (tick_type_t) 0u);
        }

        rx_buf = 0u;
        P1IE |= UART_A_RX_PIN;
    }

    os_task_terminate();
}

void uart_b_rx_task(void)
{
    static volatile uint8_t rx_buf;
    static volatile pwm_packet_t pwm_packet;

    rx_buf |= (P1IN & UART_B_RX_PIN) << 4u;
    rx_buf >>= 1;

    if (UART_BIT_COUNT == uart_b_rx_bits_pending)
    {
        os_alarm_set_rel(ALARM_F, UART_BIT_TIME_TICKS, UART_B_RX_TASK_ID, TRUE);
    }

    if (0u == --uart_b_rx_bits_pending)
    {
        os_alarm_cancel(ALARM_F);

        if (PWM_PERIOD_TICKS >= rx_buf)
        {
            // Dato completo, todos los bits recibidos y stop bit es 1.
            pwm_packet.id = UART_B_PACKET_ID;
            pwm_packet.duty_cycle = rx_buf;

            // Enviar paquete con nuevo pwm a la queue.
            os_queue_send(PWM_PACKET_QUEUE_ID, (void *) &pwm_packet, (tick_type_t) 0u);
        }

        rx_buf = 0u;
        P1IE |= UART_B_RX_PIN;
    }

    os_task_terminate();
}

void uart_c_rx_task(void)
{
    static volatile uint8_t rx_buf;
    static volatile pwm_packet_t pwm_packet;

    rx_buf |= (P1IN & UART_C_RX_PIN) << 3u;
    rx_buf >>= 1;

    if (UART_BIT_COUNT == uart_c_rx_bits_pending)
    {
        os_alarm_set_rel(ALARM_G, UART_BIT_TIME_TICKS, UART_C_RX_TASK_ID, TRUE);
    }

    if (0u == --uart_c_rx_bits_pending)
    {
        os_alarm_cancel(ALARM_G);

        if (PWM_PERIOD_TICKS >= rx_buf)
        {
            // Dato completo, todos los bits recibidos y stop bit es 1.
            pwm_packet.id = UART_C_PACKET_ID;
            pwm_packet.duty_cycle = rx_buf;

            // Enviar paquete con nuevo pwm a la queue.
            os_queue_send(PWM_PACKET_QUEUE_ID, (void *) &pwm_packet, (tick_type_t) 0u);
        }

        rx_buf = 0u;
        P1IE |= UART_C_RX_PIN;
    }

    os_task_terminate();
}

void pwm_period_task(void)
{
    static volatile uint8_t pwm_duty_cycle[3];
    static volatile pwm_packet_t * pwm_packet;
    static volatile uint8_t active_pwm;

    P2OUT |= active_pwm;

    if (0u != pwm_duty_cycle[0u] && PWM_PERIOD_TICKS != pwm_duty_cycle[0u])
    {
        os_alarm_set_rel(ALARM_B, pwm_duty_cycle[0u], PWM_R_TASK_ID, FALSE);
    }

    if (0u != pwm_duty_cycle[1u] && PWM_PERIOD_TICKS != pwm_duty_cycle[1u])
    {
        os_alarm_set_rel(ALARM_C, pwm_duty_cycle[1u], PWM_G_TASK_ID, FALSE);
    }

    if (0u != pwm_duty_cycle[2u] && PWM_PERIOD_TICKS != pwm_duty_cycle[2u])
    {
        os_alarm_set_rel(ALARM_D, pwm_duty_cycle[2u], PWM_B_TASK_ID, FALSE);
    }

    while (OS_OK == os_queue_receive(PWM_PACKET_QUEUE_ID, (void *) &pwm_packet, 0u))
    {
        pwm_duty_cycle[pwm_packet->id] = pwm_packet->duty_cycle;

        switch (pwm_packet->id)
        {
        case UART_A_PACKET_ID:
            if (0u != pwm_packet->duty_cycle)
            {
                active_pwm |= PWM_R_PIN;
            }
            else
            {
                active_pwm &= ~PWM_R_PIN;
            }
            break;
        case UART_B_PACKET_ID:
            if (0u != pwm_packet->duty_cycle)
            {
                active_pwm |= PWM_G_PIN;
            }
            else
            {
                active_pwm &= ~PWM_G_PIN;
            }
            break;
        case UART_C_PACKET_ID:
            if (0u != pwm_packet->duty_cycle)
            {
                active_pwm |= PWM_B_PIN;
            }
            else
            {
                active_pwm &= ~PWM_B_PIN;
            }
            break;
        }
    }

    os_task_terminate();
}

void pwm_r_task(void)
{
    P2OUT &= ~PWM_R_PIN;
    os_task_terminate();
}

void pwm_g_task(void)
{
    P2OUT &= ~PWM_G_PIN;
    os_task_terminate();
}

void pwm_b_task(void)
{
    P2OUT &= ~PWM_B_PIN;
    os_task_terminate();
}

#pragma vector=PORT1_VECTOR
__interrupt void virtual_uart_rx_isr(void)
{
    if (UART_A_RX_PIN & P1IFG)
    {
        P1IFG &= ~UART_A_RX_PIN;
        P1IE &= ~UART_A_RX_PIN;

        uart_a_rx_bits_pending = UART_BIT_COUNT;

        os_alarm_set_rel(ALARM_E, UART_BIT_TIME_TICKS + (UART_BIT_TIME_TICKS >> 1), UART_A_RX_TASK_ID, FALSE);
    }

    if (UART_B_RX_PIN & P1IFG)
    {
        P1IFG &= ~UART_B_RX_PIN;
        P1IE &= ~UART_B_RX_PIN;

        uart_b_rx_bits_pending = UART_BIT_COUNT;

        os_alarm_set_rel(ALARM_F, UART_BIT_TIME_TICKS + (UART_BIT_TIME_TICKS >> 1), UART_B_RX_TASK_ID, FALSE);
    }

    if (UART_C_RX_PIN & P1IFG)
    {
        P1IFG &= ~UART_C_RX_PIN;
        P1IE &= ~UART_C_RX_PIN;

        uart_c_rx_bits_pending = UART_BIT_COUNT;

        os_alarm_set_rel(ALARM_F, UART_BIT_TIME_TICKS + (UART_BIT_TIME_TICKS >> 1), UART_C_RX_TASK_ID, FALSE);
    }
}


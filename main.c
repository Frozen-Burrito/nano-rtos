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

#define UART_BIT_COUNT      ((uint8_t) 9u)
#define UART_BIT_TIME_TICKS ((uint8_t) 10u)

/* GPIO - Puerto 2 */
#define PWM_R_PIN           ((uint8_t) 0x02u)
#define PWM_G_PIN           ((uint8_t) 0x08u)
#define PWM_B_PIN           ((uint8_t) 0x20u)

#define PWM_PERIOD_TICKS    ((tick_type_t) 24u)

/* OS tasks */
#define PWM_PERIOD_TASK_ID  ((uint8_t) 0u)
#define PWM_R_TASK_ID       ((uint8_t) 1u)
#define PWM_G_TASK_ID       ((uint8_t) 2u)
#define PWM_B_TASK_ID       ((uint8_t) 3u)
#define UART_A_RX_TASK_ID   ((uint8_t) 4u)

#define PWM_PACKET_QUEUE_ID             ((queue_id_t) 0u)
#define PWM_PACKET_QUEUE_LENGTH         ((uint8_t) 4u)

typedef struct _pwm_packet_t {
    uint8_t uart_pin;
    uint8_t duty_cycle;
} pwm_packet_t;

static void pwm_r_task(void);
static void pwm_g_task(void);
static void pwm_b_task(void);
static void pwm_period_task(void);
static void uart_a_rx_task(void);
static void idle_task(void);

static uint8_t uart_a_rx_bits_pending;

int main(void)
{
	WATCHDOG_STOP;
	DCO_CAL_8MHZ();

	// Peripherals init.
    HAL_TIMER_INIT(0);

    hal_gpio_init(GPIO_PORT_1, (UART_A_RX_PIN | UART_B_RX_PIN | UART_C_RX_PIN), GPIO_DIRECTION_OUTPUT);

    hal_gpio_init(GPIO_PORT_2, (PWM_R_PIN | PWM_G_PIN | PWM_B_PIN), GPIO_DIRECTION_OUTPUT);
    hal_gpio_reset(GPIO_PORT_2, (PWM_R_PIN | PWM_G_PIN | PWM_B_PIN));

    hal_uart_init();

    hal_uart_send("Reset\r\n", 7u);

    EM_GLOBAL_INTERRUPT_EN;

    // OS init.
	os_task_create(PWM_PERIOD_TASK_ID, pwm_period_task, 2u, TRUE);
	os_task_create(PWM_R_TASK_ID, pwm_r_task, 1u, FALSE);
	os_task_create(PWM_G_TASK_ID, pwm_g_task, 1u, FALSE);
	os_task_create(PWM_B_TASK_ID, pwm_b_task, 1u, FALSE);
//	os_task_create(UART_A_RX_TASK_ID, uart_a_rx_task, 3u, FALSE);

	os_task_create(4u, idle_task, 0u, TRUE);

	os_queue_init(PWM_PACKET_QUEUE_ID, PWM_PACKET_QUEUE_LENGTH, (PWM_PERIOD_TASK_ID | UART_A_RX_TASK_ID));

	os_init();

	os_alarm_set_rel(ALARM_A, PWM_PERIOD_TICKS, PWM_PERIOD_TASK_ID, TRUE);

	scheduler_run();

	while(1);

	return 0;
}

void uart_a_rx_task(void)
{
    static uint8_t rx_buf;
    static pwm_packet_t pwm_packet;

    if (0u == --uart_a_rx_bits_pending && 0u != P1IN & UART_A_RX_PIN)
    {
        // Dato completo, todos los bits recibidos y stop bit es 1.
        pwm_packet.uart_pin = UART_A_RX_PIN;
        pwm_packet.duty_cycle = rx_buf;
        rx_buf = 0u;

        // Enviar paquete con nuevo pwm a la queue.
//        os_queue_send(PWM_PACKET_QUEUE_ID, (void *) &pwm_packet, (tick_type_t) 0u);

        P1IE |= UART_A_RX_PIN;
    }
    else
    {
        rx_buf >>= 1u;
        rx_buf |= (P1IN & UART_A_RX_PIN) << 6u;

        os_alarm_set_rel(ALARM_E, UART_BIT_TIME_TICKS, UART_A_RX_TASK_ID, FALSE);
    }

    os_task_terminate();
}

void pwm_period_task(void)
{
    hal_gpio_set(GPIO_PORT_2, (PWM_R_PIN | PWM_G_PIN | PWM_B_PIN));

    os_alarm_set_rel(ALARM_B, 1u, PWM_R_TASK_ID, FALSE);
    os_alarm_set_rel(ALARM_C, 1u, PWM_G_TASK_ID, FALSE);
    os_alarm_set_rel(ALARM_D, 1u, PWM_B_TASK_ID, FALSE);

    os_task_terminate();
}

void pwm_r_task(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_R_PIN);
    os_task_terminate();
}

void pwm_g_task(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_G_PIN);
    os_task_terminate();
}

void pwm_b_task(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_B_PIN);
    os_task_terminate();
}

void idle_task(void)
{
    while (1)
    {
        EM_SLEEP_ENTER;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void virtual_uart_rx_isr(void)
{
    if (P1IFG & UART_A_RX_PIN)
    {
        P1IFG &= ~UART_A_RX_PIN;
        P1IE &= ~UART_A_RX_PIN;

        uart_a_rx_bits_pending = UART_BIT_COUNT;

        os_alarm_set_rel(ALARM_E, UART_BIT_TIME_TICKS + (UART_BIT_TIME_TICKS >> 2), UART_A_RX_TASK_ID, FALSE);
    }
}


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

#include "os.h"

#define FALSE       ((uint8_t) 0u)
#define TRUE        ((uint8_t) 1u)

#define LED_PORT        (GPIO_PORT_1)
#define GREEN_LED_PIN   ((uint8_t) 0x01u)
#define RED_LED_PIN     ((uint8_t) 0x40u)
#define BUTTON_PIN      ((uint8_t) 0x08u)

#define LED_BLINK_DURATION_MS   ((uint16_t) 1000u)

#define TASK_A_ID       ((uint8_t) 0u)
#define TASK_B_ID       ((uint8_t) 1u)
#define TASK_C_ID       ((uint8_t) 2u)
#define IDLE_TASK_ID    ((uint8_t) 3u)

static void task_a(void);
static void task_b(void);
static void task_c(void);
static void idle_task(void);

static uint8_t led_blinks = 0u;

int main(void)
{
	WATCHDOG_STOP;
	DCO_CAL_1MHZ;

	// Peripherals init.
    HAL_TIMER_INIT();

    hal_gpio_init(LED_PORT, (GREEN_LED_PIN | RED_LED_PIN), GPIO_DIRECTION_OUTPUT);
    hal_gpio_init(GPIO_PORT_1, BUTTON_PIN, GPIO_INTERRUPT_FALLEDGE);

    hal_uart_init();

    hal_uart_send("Hello, world!\r\n");

    EM_GLOBAL_INTERRUPT_EN;

    // OS init.
	os_task_create(TASK_A_ID, task_a, 0u, TRUE);
	os_task_create(TASK_B_ID, task_b, 1u, FALSE);
//	os_task_create(TASK_C_ID, task_c, 2u, FALSE);
    os_task_create(IDLE_TASK_ID, idle_task, 0u, TRUE);

	os_init();

	scheduler();

	while(1);

	return 0;
}

void task_a(void)
{
    while (1)
    {
        EM_GLOBAL_INTERRUPT_EN;
        hal_uart_send("Task A\r\n");
        hal_gpio_reset(LED_PORT, (GREEN_LED_PIN | RED_LED_PIN));

        led_blinks = 5u;

        do {
            hal_gpio_toggle(LED_PORT, (GREEN_LED_PIN | RED_LED_PIN));
            hal_timer_delay(LED_BLINK_DURATION_MS);
        } while (led_blinks--);

//        os_task_activate(TASK_B_ID);
    }
}

void task_b(void)
{
    led_blinks = 5u;

    hal_uart_send("Task B\r\n");

    do {
        hal_gpio_toggle(LED_PORT, GREEN_LED_PIN);
        hal_timer_delay(LED_BLINK_DURATION_MS);
    } while (led_blinks--);

    os_task_terminate();
}

void task_c(void)
{
    led_blinks = 6u;

    hal_uart_send("Task C\r\n");

    do {
        hal_gpio_toggle(LED_PORT, RED_LED_PIN);
        hal_timer_delay(LED_BLINK_DURATION_MS);
    } while (led_blinks--);

    os_task_terminate();
}

void idle_task(void)
{
    hal_uart_send("Idle task\r\n");
    EM_SLEEP_ENTER;
}

#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void)
{
    if (0u == (BUTTON_PIN & P1IFG)) return;

    // SAVE_CONTEXT();

    P1IFG &= ~BUTTON_PIN;

    // Activa tarea B.
    // Cuando la tarea B termina y el scheduler regrese a tarea A (interrumpida), debe usar RESTORE_CONTEXT().
    // Task activate no activa una tarea si una ya existe con el mismo ID.
    os_task_activate_from_isr(TASK_B_ID);
}

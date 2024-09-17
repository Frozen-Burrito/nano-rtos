/*
 * main.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */
#include <stdint.h>

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

    hal_uart_init();

    hal_uart_send("Hello, world!\r\n");

    // OS init.
	os_task_create(TASK_A_ID, task_a, 0u, TRUE);
	os_task_create(TASK_B_ID, task_b, 1u, FALSE);
	os_task_create(TASK_C_ID, task_c, 2u, FALSE);
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
        hal_uart_send("Task A\r\n");
        hal_gpio_reset(LED_PORT, (GREEN_LED_PIN | RED_LED_PIN));

        led_blinks = 5u;

        do {
            hal_gpio_toggle(LED_PORT, (GREEN_LED_PIN | RED_LED_PIN));
            hal_timer_delay(LED_BLINK_DURATION_MS);
        } while (led_blinks--);

        os_task_activate(TASK_B_ID);

//        os_task_terminate();
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

    os_task_chain(TASK_C_ID);
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

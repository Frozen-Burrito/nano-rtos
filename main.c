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

#define FALSE       ((uint8_t) 0u)
#define TRUE        ((uint8_t) 1u)

/* GPIO */
// Puerto 1
#define PWM_A_PIN       ((uint8_t) 0x01u)
#define PWM_B_PIN       ((uint8_t) 0x40u)

// Puerto 2
#define PWM_C_PIN       ((uint8_t) 0x02u)
#define PWM_D_PIN       ((uint8_t) 0x08u)
#define PWM_E_PIN       ((uint8_t) 0x20u)

/* PWM Duty cycle + periodo */
#define PWM_A_DUTY_MS   ((uint16_t) 1u)
#define PWM_B_DUTY_MS   ((uint16_t) 1u)
#define PWM_C_DUTY_MS   ((uint16_t) 3u)
#define PWM_D_DUTY_MS   ((uint16_t) 4u)
#define PWM_E_DUTY_MS   ((uint16_t) 14u)

#define PWM_PERIOD_MS   ((uint16_t) 15u)

/* OS tasks */
#define PWM_TASK_A_ID       ((uint8_t) 0u)
#define PWM_TASK_B_ID       ((uint8_t) 1u)
#define PWM_TASK_C_ID       ((uint8_t) 2u)
#define PWM_TASK_D_ID       ((uint8_t) 3u)
#define PWM_TASK_E_ID       ((uint8_t) 4u)
#define PWM_PERIOD_TASK_ID  ((uint8_t) 5u)
#define IDLE_TASK_ID        ((uint8_t) 6u)

static void pwm_task_a(void);
static void pwm_task_b(void);
static void pwm_task_c(void);
static void pwm_task_d(void);
static void pwm_task_e(void);
static void pwm_period_task(void);
static void idle_task(void);

int main(void)
{
	WATCHDOG_STOP;
	DCO_CAL_8MHZ();

	// Peripherals init.
    HAL_TIMER_INIT(0);

    hal_gpio_init(GPIO_PORT_1, (PWM_A_PIN | PWM_B_PIN), GPIO_DIRECTION_OUTPUT);
    hal_gpio_init(GPIO_PORT_2, (PWM_C_PIN | PWM_D_PIN | PWM_E_PIN), GPIO_DIRECTION_OUTPUT);

    hal_uart_init();

    hal_uart_send("Reset\r\n");

    EM_GLOBAL_INTERRUPT_EN;

    // OS init.
	os_task_create(PWM_TASK_A_ID, pwm_task_a, 1u, FALSE);
	os_task_create(PWM_TASK_B_ID, pwm_task_b, 1u, FALSE);
	os_task_create(PWM_TASK_C_ID, pwm_task_c, 1u, FALSE);
	os_task_create(PWM_TASK_D_ID, pwm_task_d, 1u, FALSE);
	os_task_create(PWM_TASK_E_ID, pwm_task_e, 1u, FALSE);

	os_task_create(PWM_PERIOD_TASK_ID, pwm_period_task, 2u, TRUE);

    os_task_create(IDLE_TASK_ID, idle_task, 0u, TRUE);

    os_alarm_set_rel(ALARM_F, PWM_PERIOD_MS, PWM_PERIOD_TASK_ID, TRUE);

	os_init();

	scheduler_run();

	while(1);

	return 0;
}

void pwm_period_task(void)
{
    hal_gpio_set(GPIO_PORT_1, (PWM_A_PIN | PWM_B_PIN));
    hal_gpio_set(GPIO_PORT_2, (PWM_C_PIN | PWM_D_PIN | PWM_E_PIN));

    os_alarm_set_rel(ALARM_A, PWM_A_DUTY_MS, PWM_TASK_A_ID, FALSE);
    os_alarm_set_rel(ALARM_B, PWM_B_DUTY_MS, PWM_TASK_B_ID, FALSE);
    os_alarm_set_rel(ALARM_C, PWM_C_DUTY_MS, PWM_TASK_C_ID, FALSE);
    os_alarm_set_rel(ALARM_D, PWM_D_DUTY_MS, PWM_TASK_D_ID, FALSE);
    os_alarm_set_rel(ALARM_E, PWM_E_DUTY_MS, PWM_TASK_E_ID, FALSE);

    os_task_terminate();
}

void pwm_task_a(void)
{
    hal_gpio_reset(GPIO_PORT_1, PWM_A_PIN);

    os_task_terminate();
}

void pwm_task_b(void)
{
    hal_gpio_reset(GPIO_PORT_1, PWM_B_PIN);

    os_task_terminate();
}

void pwm_task_c(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_C_PIN);

    os_task_terminate();
}

void pwm_task_d(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_D_PIN);

    os_task_terminate();
}

void pwm_task_e(void)
{
    hal_gpio_reset(GPIO_PORT_2, PWM_E_PIN);

    os_task_terminate();
}

void idle_task(void)
{
    while (1)
    {
        hal_uart_send("Idle task\r\n");
        EM_SLEEP_ENTER;
    }
}

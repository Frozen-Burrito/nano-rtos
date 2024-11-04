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

#include "os/os.h"
#include "os/tasks.h"
#include "os/alarms.h"
#include "os/queues.h"

#define FALSE       ((uint8_t) 0u)
#define TRUE        ((uint8_t) 1u)

/* GPIO */
#define GREEN_LED_PIN       ((uint8_t) 0x01u)
#define RED_LED_PIN         ((uint8_t) 0x40u)

/* OS tasks */
#define PRODUCER_TASK_ID    ((uint8_t) 0u)
#define CONSUMER_TASK_ID    ((uint8_t) 1u)
#define ANOTHER_TASK_ID     ((uint8_t) 2u)

#define TEST_QUEUE_ID       ((queue_id_t) 0u)
#define TEST_QUEUE_LENGTH   ((uint8_t) 3u)

#define INITIAL_PRODUCER_DELAY_TICKS    ((tick_type_t) 100u)
#define ANOTHER_TASK_DELAY_TICKS        ((tick_type_t) 5000u)

static void producer_task(void);
static void consumer_task(void);
static void another_task(void);

int main(void)
{
	WATCHDOG_STOP;
	DCO_CAL_8MHZ();

	// Peripherals init.
    HAL_TIMER_INIT(0);

    hal_gpio_init(GPIO_PORT_1, (GREEN_LED_PIN | RED_LED_PIN), GPIO_DIRECTION_OUTPUT);

    hal_uart_init();

    hal_uart_send("Reset\r\n", 7u);

    EM_GLOBAL_INTERRUPT_EN;

    // OS init.
	os_task_create(PRODUCER_TASK_ID, producer_task, 3u, FALSE);
	os_task_create(CONSUMER_TASK_ID, consumer_task, 3u, TRUE);
	os_task_create(ANOTHER_TASK_ID, another_task, 4u, FALSE);

    // Solo las tareas productora y consumidora tienen permiso para acceder a TEST_QUEUE.
    os_queue_init(TEST_QUEUE_ID, TEST_QUEUE_LENGTH, (0x01u << PRODUCER_TASK_ID) | (0x01u << CONSUMER_TASK_ID));

    // Delays iniciales para probar queues.
    os_alarm_set_rel(ALARM_A, INITIAL_PRODUCER_DELAY_TICKS, PRODUCER_TASK_ID, FALSE);
    os_alarm_set_rel(ALARM_B, ANOTHER_TASK_DELAY_TICKS, ANOTHER_TASK_ID, FALSE);

	os_init();

	scheduler_run();

	while(1);

	return 0;
}

void producer_task(void)
{
    static error_id_e status;
    static uint8_t failed_attempts_remaining = 5u;
    static uint8_t msg[] = "Hola,mundo";
    static uint8_t msg_cursor;
    static uint8_t send_pass_log[] = "SP val = _\r\n";

    while (1)
    {
        status = os_queue_send(TEST_QUEUE_ID, (void *) &(msg[msg_cursor]), (tick_type_t) 20u);

        hal_timer_delay(200u);

        if (OS_OK == status)
        {
            send_pass_log[9] = msg[msg_cursor];
            hal_uart_send(send_pass_log, 12u);
            send_pass_log[9] = '_';

            msg_cursor++;
            if (10 <= msg_cursor)
            {
                msg_cursor = 0u;
            }

            failed_attempts_remaining = 5u;
        }
        else
        {
            hal_uart_send("SF FULL, 20\r\n", 13u);
            failed_attempts_remaining--;

            if (0u == failed_attempts_remaining)
            {
                break;
            }
        }
    }

    os_task_terminate();
}

void consumer_task(void)
{
    static error_id_e status;
    static uint8_t * item;
    static uint8_t count;
    static uint8_t rx_success_msg[] = {'R', 'P', ' ', '_', '\r', '\n'};

    // Primer receive falla si producer_task no envía algo inmediatamente.
    // ticks_to_wait es 0, entonces consumer_task no pasa a estado WAIT.
    status = os_queue_receive(TEST_QUEUE_ID, (void *) &item, (tick_type_t) 0u);

    if (OS_OK == status)
    {
        rx_success_msg[3] = *item;
        hal_uart_send(rx_success_msg, 6u);
        rx_success_msg[3] = '_';
    }
    else
    {
        hal_uart_send("RF, EMPTY\r\n", 11u);
    }

    // consumer_task espera indeterminadamente a que producer_task envíe algo en la queue.
    status = os_queue_receive(TEST_QUEUE_ID, (void *) &item, OS_MAX_TICKS);
    // Si la tarea llega a este hal_uart_send(), significa que recibió un elemento de la queue.
    rx_success_msg[3] = *item;
    hal_uart_send(rx_success_msg, 6u);
    rx_success_msg[3] = '0';

    // Recibir 5 elementos, luego dejar de recibir.
    count = 5u;
    while (count--)
    {
        status = os_queue_receive(TEST_QUEUE_ID, (void *) &item, (tick_type_t) 1000u);

        if (OS_OK == status)
        {
            rx_success_msg[3] = *item;
            hal_uart_send(rx_success_msg, 6u);
            rx_success_msg[3] = '0';
        }
        else
        {
            hal_uart_send("RF EMPTY, 30\r\n", 14u);
        }
    }

    os_task_terminate();
}

void another_task(void)
{
    static error_id_e status;
    static uint8_t item = 'Z';

    status = os_queue_send(TEST_QUEUE_ID, (void *) &item, (tick_type_t) 0u);

    // another_task no tiene permiso para usar TEST_QUEUE.
    // os_queue_send debe fallar con OS_ERROR_INVALID_ARGUMENT.
    if (OS_ERROR_INVALID_ARGUMENT == status)
    {
        hal_uart_send("Send missing permission\r\n", 25u);
    }
    else
    {
        hal_uart_send("Another task send OK\r\n", 22u);
    }

    os_task_terminate();
}

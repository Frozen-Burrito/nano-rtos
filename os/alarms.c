/*
 * alarms.c
 *
 *  Created on: Sep 24, 2024
 *      Author: Fernando Mendoza V.
 */
#include <msp430.h>

#include "alarms.h"

#include "os_private.h"

#ifdef SYSTICK_BASE_TA0_0
#define SYSTICK_VECTOR  TIMER0_A0_VECTOR
#define SYSTICK_CCR     TA0CCR0
#endif /* SYSTICK_BASE_TA0_0 */

#define ALARM_AUTORELOAD    (0x01u)
#define ALARM_ACTIVE        (0x02u)

typedef struct _alarm_t {
    uint16_t ticks;
    uint16_t count;
    uint8_t task_to_activate;
    uint8_t state;
} alarm_t;

static volatile alarm_t alarms[ALARM_MAX];

error_id_e os_alarm_set_rel(alarm_id_e id, uint16_t ticks, task_id_t task_to_activate, uint8_t autoreload)
{
    if (ALARM_MAX <= id || 0 == ticks || NUM_TASK_MAX < task_to_activate)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    alarms[id] = (alarm_t) {
        .ticks = ticks,
        .count = ticks,
        .task_to_activate = task_to_activate,
        .state = ALARM_ACTIVE,
    };

    if (autoreload)
    {
        alarms[id].state |= ALARM_AUTORELOAD;
    }

    return OS_OK;
}

error_id_e os_alarm_cancel(alarm_id_e id)
{
    if (ALARM_MAX <= id)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    alarms[id].state &= ~ALARM_ACTIVE;

    return OS_OK;
}

#pragma vector=SYSTICK_VECTOR
__interrupt void systick_isr(void)
{
    volatile uint16_t i = ALARM_MAX;
    volatile uint8_t task_activated = 0;

    SYSTICK_CCR += SYSTICK_PERIOD;

    while (i--)
    {
        if (ALARM_ACTIVE & alarms[i].state && 0u == (--alarms[i].count))
        {
            if (OS_TASK_STATE_EMPTY != tasks[alarms[i].task_to_activate].state)
            {
                tasks[alarms[i].task_to_activate].state = OS_TASK_STATE_READY;
                num_active_tasks++;
            }

            if (ALARM_AUTORELOAD & alarms[i].state)
            {
                alarms[i].count = alarms[i].ticks;
            }
            else
            {
                alarms[i].state &= ~ALARM_ACTIVE;
            }

            task_activated = 1;
        }
    }

    if (task_activated)
    {
        if (OS_TASK_ID_MAX != current_task)
        {
            tasks[current_task].state = OS_TASK_STATE_READY;
        }

        // Descartar espacio de stack usado por variables locales i y task_activated.
        __asm volatile (" ADD #4, SP");
        // Recuperar valores originales de R11 -> R15 desde el stack (la ISR los mueve cuando inicia).
        __asm volatile (" POP R10");
        __asm volatile (" POP R11");
        __asm volatile (" POP R12");
        __asm volatile (" POP R13");
        __asm volatile (" POP R14");
        __asm volatile (" POP R15");
        // Desactivar interrupciones en SR guardado y luego sacarlo del stack.
        __asm volatile (" BIC #0x00F8, 0(SP)");
        __asm volatile (" POP SR");
        SAVE_CONTEXT();

        scheduler_run();
    }
}


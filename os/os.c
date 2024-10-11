/*
 * os.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#include "os.h"

#include <msp430.h>

#define NUM_TASK_MAX    ((uint8_t) 7u)
#define OS_TASK_ID_MAX  ((uint8_t) 0xFFu)
#define TASK_STACK_SIZE ((uint8_t) 16u)

#define ALARM_AUTORELOAD    (0x01u)
#define ALARM_ACTIVE        (0x02u)

// @brief Guarda todos los registros del contexto de la tarea actual, excepto R4.
// Recuperar R0 (PC) del stack.
// Mover puntero de stack de la tarea a R4.
// Almacenar valor original de R0 en stack de la tarea.
// Almacenar R1 -> R15 en stack de la tarea.
#define SAVE_CONTEXT() ({\
    __asm volatile (" MOV &current_task_stack, R4");\
    __asm volatile (" MOV 0(SP), 0(R4)");\
    __asm volatile (" ADD #2, SP");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV SP, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R2, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R3, 0(R4)");\
    __asm volatile (" SUB #4, R4");\
    __asm volatile (" MOV R5, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R6, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R7, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R8, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R9, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R10, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R11, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R12, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R13, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R14, 0(R4)");\
    __asm volatile (" SUB #2, R4");\
    __asm volatile (" MOV R15, 0(R4)");\
    __asm volatile (" MOV R4, &current_task_stack");\
})


// @brief Recupera todos los registros de la tarea, excepto R4.
// Mover puntero de stack de la tarea a R4.
// Recuperar valor original de R15 -> R5.
// Ignorar valor original de R4.
// Recuperar valor original de R3 -> R0 (PC).
#define RESTORE_CONTEXT() ({\
    __asm volatile (" MOV &current_task_stack, R4");\
    __asm volatile (" MOV @R4+, R15");\
    __asm volatile (" MOV @R4+, R14");\
    __asm volatile (" MOV @R4+, R13");\
    __asm volatile (" MOV @R4+, R12");\
    __asm volatile (" MOV @R4+, R11");\
    __asm volatile (" MOV @R4+, R10");\
    __asm volatile (" MOV @R4+, R9");\
    __asm volatile (" MOV @R4+, R8");\
    __asm volatile (" MOV @R4+, R7");\
    __asm volatile (" MOV @R4+, R6");\
    __asm volatile (" MOV @R4+, R5");\
    __asm volatile (" ADD #2, R4");\
    __asm volatile (" MOV @R4+, R3");\
    __asm volatile (" ADD #2, R4");\
    __asm volatile (" BIC #16, 0(R4)");\
    __asm volatile (" MOV @R4+, SR");\
    __asm volatile (" MOV @R4+, SP");\
    __asm volatile (" MOV R4, &current_task_stack");\
    __asm volatile (" EINT");\
    __asm volatile (" MOV @R4, PC");\
})

typedef enum _task_state_e {
    OS_TASK_STATE_EMPTY,
    OS_TASK_STATE_SUSPENDED,
    OS_TASK_STATE_WAIT,
    OS_TASK_STATE_READY,
    OS_TASK_STATE_RUN,
} task_state_e;

typedef struct _task_t {
    task_state_e state;                 /* Estado actual de la tarea. */
    task_function_t task_function;      /* Dirección de inicio de la tarea. */
    uint8_t priority;                   /* Prioridad, en rango 0-255. */
    uint8_t autostart;                  /* Si es TRUE, la inicialización del sistema activa la tarea automáticamente. */
    uint16_t stack[TASK_STACK_SIZE];    /* Memoria para guardar el contexto de la tarea (R0-R15). */
} task_t;

typedef struct _alarm_t {
    uint16_t ticks;
    uint16_t count;
    uint8_t task_to_activate;
    uint8_t state;
} alarm_t;

static task_t tasks[NUM_TASK_MAX];

static task_id_t current_task = OS_TASK_ID_MAX;
static uint8_t num_active_tasks = 0u;

volatile uint16_t * current_task_stack;
volatile uint16_t temp_register_value;

static volatile alarm_t alarms[ALARM_MAX];

error_id_e os_init(void)
{
    volatile uint8_t i;

    i = NUM_TASK_MAX;
    while (i--)
    {
        // Cargar valor inicial de PC para cada tarea.
        tasks[i].stack[TASK_STACK_SIZE - 1u] = (uint16_t) tasks[i].task_function;

        // Inicar tareas con autostart.
        if (OS_TASK_STATE_SUSPENDED == tasks[i].state && tasks[i].autostart)
        {
            tasks[i].state = OS_TASK_STATE_READY;
            num_active_tasks++;
        }
    }

    // Iniciar timer para alarmas.
    TA0CCR2 += 1000u;
    TA0CCTL2 |= CCIE;

    return OS_OK;
}

error_id_e os_task_create(task_id_t task_id, task_function_t task_function, uint8_t priority, uint8_t autostart)
{
    error_id_e status = OS_OK;

    if (NUM_TASK_MAX < task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        tasks[task_id] = (task_t) {
            .state = OS_TASK_STATE_SUSPENDED,
            .task_function = task_function,
            .priority = priority,
            .autostart = autostart,
        };
    }

    return status;
}

error_id_e os_task_activate(task_id_t task_id)
{
    //TODO: Revisar esto cuando no hay una tarea ejecutandose.
    SAVE_CONTEXT();

    volatile error_id_e status = OS_OK;

    if (NUM_TASK_MAX <= task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (NUM_TASK_MAX == num_active_tasks)
    {
        status = OS_ERROR_MAX_ACTIVE_TASKS;
    }

    if (OS_OK == status)
    {
        if (OS_TASK_ID_MAX != current_task)
        {
            tasks[current_task].state = OS_TASK_STATE_READY;
        }

        tasks[task_id].state = OS_TASK_STATE_READY;
        num_active_tasks++;

        scheduler();
    }
    return status;
}

error_id_e os_task_activate_from_isr(task_id_t task_id)
{
    if (NUM_TASK_MAX <= task_id || OS_TASK_STATE_SUSPENDED != tasks[task_id].state)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    if (NUM_TASK_MAX == num_active_tasks)
    {
        return OS_ERROR_MAX_ACTIVE_TASKS;
    }

    if (OS_TASK_ID_MAX != current_task)
    {
        tasks[current_task].state = OS_TASK_STATE_READY;
    }

    tasks[task_id].state = OS_TASK_STATE_READY;
    num_active_tasks++;

    __asm volatile (" ADD #2, SP");
    // Recuperar valores originales de R11 -> R15 desde el stack (la ISR los mueve cuando inicia).
    __asm volatile (" POP R11");
    __asm volatile (" POP R12");
    __asm volatile (" POP R13");
    __asm volatile (" POP R14");
    __asm volatile (" POP R15");
    __asm volatile (" POP R2");
    SAVE_CONTEXT();

    scheduler();

    return OS_OK;
}

error_id_e os_task_terminate(void)
{
    tasks[current_task].state = OS_TASK_STATE_SUSPENDED;
    tasks[current_task].stack[TASK_STACK_SIZE - 2u] = 0u;
    tasks[current_task].stack[TASK_STACK_SIZE - 1u] = (uint16_t) tasks[current_task].task_function;

    num_active_tasks--;

    scheduler();

    return OS_OK;
}

error_id_e os_task_chain(task_id_t task_id)
{
    SAVE_CONTEXT();

    volatile error_id_e status = OS_OK;

    if (NUM_TASK_MAX <= task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        tasks[current_task].state = OS_TASK_STATE_SUSPENDED;

        tasks[task_id].state = OS_TASK_STATE_READY;

        scheduler();
    }

    return status;
}

error_id_e os_alarm_set_rel(alarm_id_e id, uint16_t ticks, uint8_t task_to_activate, uint8_t autoreload)
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

void scheduler(void)
{
    volatile uint8_t top_priority = 0u;
    volatile uint8_t top_priority_task_id = OS_TASK_ID_MAX;
    volatile task_id_t i;

    if (0u != num_active_tasks)
    {
        i = NUM_TASK_MAX;
        while (i--)
        {
            if (OS_TASK_STATE_READY == tasks[i].state && top_priority <= tasks[i].priority)
            {
                top_priority = tasks[i].priority;
                top_priority_task_id = i;
            }
        }
    }

    if (OS_TASK_ID_MAX != top_priority_task_id)
    {
        current_task = top_priority_task_id;

        tasks[current_task].state = OS_TASK_STATE_RUN;

        current_task_stack = (uint16_t *) tasks[current_task].stack;

        // Recuperar 3 espacios de 16 bits usados por variables locales y direccion de retorno.
        __asm volatile (" ADD #6, SP");

        // Workaround para problemas con asignacion de stack global por tarea.
        if (0u == current_task_stack[TASK_STACK_SIZE - 2u])
        {
            __asm volatile (" MOV SP, temp_register_value");
            current_task_stack[TASK_STACK_SIZE - 2u] = temp_register_value;
        }

        RESTORE_CONTEXT();
    }
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer_a0_taifg_isr(void)
{
    volatile uint16_t i;
    volatile uint8_t task_activated;

    if (TA0IV_TACCR2 & TA0IV)
    {
        task_activated = 0;
        TA0CCR2 += 1000u;

        i = ALARM_MAX;
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

            scheduler();
        }
    }
}

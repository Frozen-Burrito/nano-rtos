/*
 * os.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#include "os.h"

#define NUM_TASK_MAX    ((uint8_t) 5u)
#define OS_TASK_ID_MAX  ((uint8_t) 0xFFu)
#define TASK_STACK_SIZE ((uint8_t) 16u)

// @brief Guarda todos los registros del contexto de la tarea actual, excepto R4.
// Recuperar R0 (PC) del stack.
// Mover puntero de stack de la tarea a R4.
// Almacenar valor original de R0 en stack de la tarea.
// Almacenar R1 -> R15 en stack de la tarea.
#define SAVE_CONTEXT() ({\
    __asm volatile (" NOP");\
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
    __asm volatile (" MOV @R4+, SR");\
    __asm volatile (" MOV @R4+, SP");\
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

static task_t tasks[NUM_TASK_MAX];

static task_id_t current_task = OS_TASK_ID_MAX;
static uint8_t num_active_tasks = 0u;

volatile uint16_t * current_task_stack;
volatile uint16_t temp_register_value;

error_id_e os_init(void)
{
    volatile uint8_t i;

    for (i = NUM_TASK_MAX; i != 0u; i--)
    {
        // Inicializar valores de PC, SP y SR en contexto de la tarea.
        tasks[i - 1u].stack[TASK_STACK_SIZE - 1u] = (uint16_t) tasks[i - 1u].task_function;
        __asm volatile (" MOV SP, temp_register_value");
        // El 4u para compensar el espacio de stack usado por os_init().
        tasks[i - 1u].stack[TASK_STACK_SIZE - 2u] = temp_register_value + 4u;

        if (OS_TASK_STATE_SUSPENDED == tasks[(uint8_t)(i - 1)].state && tasks[(uint8_t)(i - 1)].autostart)
        {
            tasks[(uint8_t)(i - 1)].state = OS_TASK_STATE_READY;
            num_active_tasks++;
        }
    }

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
    num_active_tasks--;

    scheduler();

    return OS_OK;
}

error_id_e os_task_chain(task_id_t task_id)
{
    SAVE_CONTEXT();

    volatile error_id_e status = OS_OK;

    SAVE_CONTEXT();

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

void scheduler(void)
{
    uint8_t top_priority = 0u;
    uint8_t top_priority_task_id = OS_TASK_ID_MAX;
    task_id_t i;

    if (0u != num_active_tasks)
    {
        for (i = NUM_TASK_MAX; 0u != i; i--)
        {
            if (OS_TASK_STATE_READY == tasks[(uint8_t) (i - 1)].state && top_priority <= tasks[(uint8_t) (i - 1)].priority)
            {
                top_priority = tasks[(uint8_t) (i - 1)].priority;
                top_priority_task_id = (uint8_t) (i - 1);
            }
        }
    }

    if (OS_TASK_ID_MAX != top_priority_task_id)
    {
        current_task = top_priority_task_id;

        tasks[current_task].state = OS_TASK_STATE_RUN;

        current_task_stack = (uint16_t *) tasks[current_task].stack;

        RESTORE_CONTEXT();
    }
}

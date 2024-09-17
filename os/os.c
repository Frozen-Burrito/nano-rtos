/*
 * os.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#include "os.h"

#define NUM_TASK_MAX    ((uint8_t) 10u)
#define OS_TASK_ID_MAX  ((uint8_t) 0xFFu)

typedef enum _task_state_e {
    OS_TASK_STATE_EMPTY,
    OS_TASK_STATE_SUSPENDED,
    OS_TASK_STATE_WAIT,
    OS_TASK_STATE_READY,
    OS_TASK_STATE_RUN,
} task_state_e;

typedef struct _task_t {
    task_state_e state;             /* Estado actual de la tarea. */
    task_function_t task_function;  /* Dirección de inicio de la tarea. */
    uint8_t priority;               /* Prioridad, en rango 0-255. */
    uint8_t autostart;              /* Si es TRUE, la inicialización del sistema activa la tarea automáticamente. */
    uint16_t task_pc;               /* Valor de PC la última vez que fue cambiada esta tarea. */
} task_t;

static task_t tasks[NUM_TASK_MAX];

static task_id_t current_task = OS_TASK_ID_MAX;
static uint8_t num_active_tasks = 0u;

void (*task_pc)(void);

error_id_e os_init(void)
{
    uint8_t i;

    for (i = NUM_TASK_MAX; i != 0u; i--)
    {
        tasks[(uint8_t)(i - 1)].task_pc = (uint16_t) tasks[(uint8_t)(i - 1)].task_function;

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
            .task_pc = (uint16_t) task_function,
        };
    }

    return status;
}

error_id_e os_task_activate(task_id_t task_id)
{
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

            // Guardar PC de la tarea que invoca a task_activate.
            __asm ("L1: MOV 2(SP), task_pc");
            __asm ("L2: ADD #4, SP");
            tasks[current_task].task_pc = (uint16_t) task_pc;
        }

        tasks[task_id].state = OS_TASK_STATE_READY;
        tasks[task_id].task_pc = (uint16_t) tasks[task_id].task_function;
        num_active_tasks++;

        scheduler();
    }

    // Considerar guardar PC de "return status"
    // Considerar forma de que tarea que invoca reciba el valor de status.

    return status;
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
    volatile error_id_e status = OS_OK;

    if (NUM_TASK_MAX <= task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        tasks[current_task].state = OS_TASK_STATE_SUSPENDED;
        // Guardar PC de la tarea que invoca a task_activate.
        __asm ("L4: MOV 2(SP), task_pc");
        __asm ("L5: ADD #2, SP");
        tasks[current_task].task_pc = (uint16_t) task_pc;

        tasks[task_id].state = OS_TASK_STATE_READY;
        tasks[task_id].task_pc = (uint16_t) tasks[task_id].task_function;

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

        task_pc = (task_function_t) tasks[current_task].task_pc;
        __asm ("L6: ADD #6, SP");
        __asm ("L7: MOV task_pc, PC");
    }
}

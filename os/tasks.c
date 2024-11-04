/*
 * tasks.c
 *
 *  Created on: Oct 13, 2024
 *      Author: Fernando Mendoza V.
 */
#include "tasks.h"

#include "os_private.h"

volatile task_t tasks[NUM_TASK_MAX];

volatile task_id_t current_task = OS_TASK_ID_MAX;
volatile uint8_t num_active_tasks = 0u;

error_id_e os_task_create(os_task_id_t task_id, os_task_function_t task_function, uint8_t priority, uint8_t autostart)
{
    error_id_e status = OS_OK;

    if (NUM_TASK_MAX <= task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        tasks[(task_id_t) task_id] = (task_t) {
            .state = OS_TASK_STATE_SUSPENDED,
            .task_function = (task_function_t) task_function,
            .priority = priority,
            .autostart = autostart,
        };
    }

    return status;
}

error_id_e os_task_activate(os_task_id_t task_id)
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
        }

        tasks[task_id].state = OS_TASK_STATE_READY;
        num_active_tasks++;

        scheduler_run();
    }
    return status;
}

error_id_e os_task_activate_from_isr(os_task_id_t task_id)
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

    scheduler_run();

    return OS_OK;
}

error_id_e os_task_terminate(void)
{
    tasks[current_task].state = OS_TASK_STATE_SUSPENDED;
    tasks[current_task].stack[TASK_STACK_SIZE - 2u] = 0u;
    tasks[current_task].stack[TASK_STACK_SIZE - 1u] = (uint16_t) tasks[current_task].task_function;

    num_active_tasks--;

    scheduler_run();

    return OS_OK;
}

error_id_e os_task_chain(os_task_id_t task_id)
{
    volatile error_id_e status = OS_OK;

    if (NUM_TASK_MAX <= task_id)
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        tasks[current_task].state = OS_TASK_STATE_SUSPENDED;

        tasks[task_id].state = OS_TASK_STATE_READY;

        scheduler_run();
    }

    return status;
}

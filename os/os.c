/*
 * os.c
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#include "os.h"

#include "os_private.h"

#ifdef SYSTICK_BASE_TA0_0
#define SYSTICK_TIMER_ENABLE    HAL_TIMER_A0_0_START(SYSTICK_PERIOD)
#endif

volatile uint16_t temp_register_value;

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
    SYSTICK_TIMER_ENABLE;

    return OS_OK;
}

void scheduler_run(void)
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

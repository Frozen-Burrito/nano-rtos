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
volatile uint8_t scheduler_from_isr;

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

    // TODO: Pruebas temporales para manejo de stack. Hace falta integrar esto con el servicio create_task.
    // 40 bytes de stack por tarea.
    tasks[0].stack[TASK_STACK_SIZE - 2u] = 0x03FE;
    tasks[1].stack[TASK_STACK_SIZE - 2u] = 0x03DE;
    tasks[2].stack[TASK_STACK_SIZE - 2u] = 0x03CE;
    tasks[3].stack[TASK_STACK_SIZE - 2u] = 0x03B6;
    tasks[4].stack[TASK_STACK_SIZE - 2u] = 0x03AE;

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
        tasks[top_priority_task_id].state = OS_TASK_STATE_RUN;

        if (current_task != top_priority_task_id)
        {
            if (OS_TASK_ID_MAX != current_task && OS_TASK_STATE_READY == tasks[current_task].state)
            {
                SAVE_CONTEXT();
            }

            current_task = top_priority_task_id;
            current_task_stack = (uint16_t *) tasks[current_task].stack;

            // Recuperar 3 espacios de 16 bits usados por variables locales y direccion de retorno.
            __asm volatile (" ADD #6, SP");

            RESTORE_CONTEXT();
        }
    }
#if OS_CUSTOM_IDLE_TASK == 0
    else
    {
        EM_SLEEP_ENTER;
    }
#endif
}


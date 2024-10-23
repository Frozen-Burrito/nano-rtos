/*
 * queues.c
 *
 *  Created on: Oct 17, 2024
 *      Author: Fernando Mendoza V.
 */
#include "queues.h"

#include "os_private.h"

#include "em.h"

// Workaround temporal. Queues usan n-1 espacios, desperdiciando el último. Modificar algoritmo de queues para aprovecharlo.
//TODO: Modificar send y receive para que usen todos los elementos de queue_t.data, en vez de length - 1.
typedef struct _queue_t {
    uint16_t data[OS_QUEUE_LEN_MAX + 1u];   /* Arreglo de punteros a los datos. */
    uint16_t tasks_waiting;                 /* MSB son tareas esperando espacio libre, LSB son tareas esperando al menos un elemento. */
    uint8_t length;                         /* Número de elementos que puede contener la queue. */
    uint8_t tasks_with_access;              /* Campo de bits donde un bit set indica que la tarea con el ID correspondiente tiene acceso a la queue. */
    uint8_t head;                           /* Índice del primer elemento de la queue. */
    uint8_t tail;                           /* Índice del elemento final de la queue. */
} queue_t;

static queue_t queues[OS_QUEUE_COUNT_MAX];

error_id_e os_queue_init(queue_id_t id, uint8_t length, uint8_t access)
{
    if (OS_QUEUE_COUNT_MAX <= id || 0u == length || 0u == access)
    {
        return OS_ERROR_INVALID_ARGUMENT;
    }

    queues[id] = (queue_t) {
        .length = length + 1,               /*TODO: Eliminar + 1*/
        .tasks_with_access = access,
        .tail = 0u,
        .head = 0u,
    };

    return OS_OK;
}

error_id_e os_queue_send(queue_id_t id, const void * item, tick_type_t ticks_to_wait)
{
    volatile register error_id_e status = OS_OK;
    volatile register uint8_t i;

    EM_GLOBAL_INTERRUPT_DIS;

    if (OS_QUEUE_COUNT_MAX <= id || 0u == (queues[id].tasks_with_access & (1u << current_task)))
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        i = queues[id].tail + 1;
        if (queues[id].length <= i)
        {
            i = 0u;
        }

        if (i != queues[id].head)
        {
            // Queue tiene espacio libre, agregar elemento al final.
            queues[id].data[queues[id].tail] = (uint16_t) item;
            queues[id].tail = i;
        }
        else if (0u != ticks_to_wait)
        {
            // Queue está llena. Tarea debe esperar al timeout o a que liberen espacio de la queue.
            queues[id].tasks_waiting |= ((uint16_t) (0xFFu & (1u << current_task))) << 8u;
            tasks[current_task].state = OS_TASK_STATE_WAIT;
            tasks[current_task].ticks_to_wait = ticks_to_wait;

            scheduler_run();

            status = OS_OK;
            i = queues[id].tail + 1;
            if (queues[id].length <= i)
            {
                i = 0u;
            }

            // Tarea pasó a estado wait por al menos un tick. Llega a este punto si expiró el timeout o se liberó espacio.
            queues[id].tasks_waiting &= ~(((uint16_t) (0xFFu & (1u << current_task))) << 8u);

            if (i != queues[id].head)
            {
                // Se liberó espacio en la queue mientras la tarea esperaba.
                queues[id].data[queues[id].tail] = (uint16_t) item;
                queues[id].tail = i;
            }
            else
            {
                status = OS_ERROR_QUEUE_FULL;
            }
        }
        else
        {
            status = OS_ERROR_QUEUE_FULL;
        }
    }

    // Revisar si el nuevo elemento agregado desbloquea alguna tarea (queue ya no está vacía).
    if (OS_OK == status)
    {
        i = NUM_TASK_MAX;
        while (i--)
        {
            if (0u != (queues[id].tasks_waiting & ((uint16_t) (0xFFu & (1u << i)))))
            {
                // La tarea i estaba esperando a que la queue no estuviera vacía. Pasarla a estado ready.
                tasks[(task_id_t) i].state = OS_TASK_STATE_READY;
                tasks[(task_id_t) i].ticks_to_wait = 0u;
            }
        }
    }

    EM_GLOBAL_INTERRUPT_EN;

    return status;
}

error_id_e os_queue_receive(queue_id_t id, void * out_item, tick_type_t ticks_to_wait)
{
    volatile register error_id_e status = OS_OK;
    volatile register uint8_t i;

    EM_GLOBAL_INTERRUPT_DIS;

    if (OS_QUEUE_COUNT_MAX <= id || 0u == (queues[id].tasks_with_access & (1u << current_task)))
    {
        status = OS_ERROR_INVALID_ARGUMENT;
    }

    if (OS_OK == status)
    {
        if (queues[id].head != queues[id].tail)
        {
            // Queue tiene al menos un elemento. Remover el elemento de la queue.
            i = queues[id].head;
            *((uint16_t *) out_item) = queues[id].data[i];

            i++;
            if (queues[id].length <= i)
            {
                i = 0u;
            }
            queues[id].head = i;
        }
        else if (0u != ticks_to_wait)
        {
            // Queue está vacía y timeout no es 0. Esperar a que queue tenga un elemento o al timeout.
            queues[id].tasks_waiting |= ((uint16_t) (0xFFu & (1u << current_task)));
            tasks[current_task].state = OS_TASK_STATE_WAIT;
            tasks[current_task].ticks_to_wait = ticks_to_wait;

            scheduler_run();

            status = OS_OK;
            i = queues[id].head;

            // Tarea pasó a estado wait por al menos un tick. Llega a este punto si expiró el timeout o queue recibió un elemento.
            queues[id].tasks_waiting &= ~((uint16_t) (0xFFu & (1u << current_task)));

            if (queues[id].head != queues[id].tail)
            {
                // La queue recibió un elemento mientras la tarea esperaba.
                i = queues[id].head;
                *((uint16_t *) out_item) = queues[id].data[i];

                i++;
                if (queues[id].length <= i)
                {
                    i = 0u;
                }
                queues[id].head = i;
            }
            else
            {
                status = OS_ERROR_QUEUE_EMPTY;
            }
        }
        else
        {
            status = OS_ERROR_QUEUE_EMPTY;
        }
    }

    // Revisar si remover el elemento desbloquea alguna tarea (queue ya no está llena).
    if (OS_OK == status)
    {
        i = NUM_TASK_MAX;
        while (i--)
        {
            if (0u != (queues[id].tasks_waiting & (((uint16_t) (0xFFu & (1u << i))) << 8u)))
            {
                // La tarea i estaba esperando a que la queue no estuviera llena. Pasar la tarea i a estado ready.
                tasks[(task_id_t) i].state = OS_TASK_STATE_READY;
                tasks[(task_id_t) i].ticks_to_wait = 0u;
            }
        }
    }

    EM_GLOBAL_INTERRUPT_EN;

    return status;
}


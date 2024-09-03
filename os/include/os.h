/*
 * os.h
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_H_
#define OS_H_

#include <stdint.h>

#include "em.h"

typedef uint8_t task_id_t;
typedef void (*task_function_t)(void);

typedef enum _error_id_e {
    OS_OK,                          /* Ning�n error, funci�n complet� con �xito. */
    OS_ERROR_INVALID_ARGUMENT,      /* Uno o varios de los argumentos de una funci�n son no-v�lidos. */
    OS_ERROR_MAX_CREATED_TASKS,     /* El OS alcanz� el l�mite de tareas creadas. No se puede crear una nueva tarea. */
    OS_ERROR_MAX_ACTIVE_TASKS,      /* El OS ya tiene el m�ximo n�mero de tareas activas. */
} error_id_e;

/**
 * @brief Inicializa el sistema operativo y ejecuta el scheduler por primera vez.
 *
 * @return OS_OK si inicializ� OS sin errores.
 */
error_id_e os_init(void);

/**
 * @brief Registra los atributos de una nueva tarea.
 *
 * @param task_id Identificador �nico de la tarea.
 * @param task_function Direcci�n de inicio de la tarea.
 * @param priority Prioridad de la tarea (0-255).
 * @param autostart Si es TRUE, inicializar el OS activa autom�ticamente la tarea.
 *
 * @return OS_OK si registr� la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador v�lido.
 *         OS_ERROR_MAX_CREATED_TASKS si el OS alcanz� el l�mite de tareas creadas.
 */
error_id_e os_task_create(task_id_t task_id, task_function_t task_function, uint8_t priority, uint8_t autostart);

/**
 * @brief Activa una tarea para que pueda ser elegida por el scheduler.
 *
 * @param task_id Identificador �nico de la tarea que ser� activada.
 *
 * @return OS_OK si activ� la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador v�lido.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanz� el l�mite de tareas activas.
 */
error_id_e os_task_activate(task_id_t task_id);

/**
 * @brief Suspende una tarea.
 *
 * La tarea no vuelve a ser ejecutada hasta que se active otra vez.
 *
 * @return OS_OK si suspendi� la tarea.
 */
error_id_e os_task_terminate(void);

/**
 * @brief Suspende la tarea actual y activa otra.
 *
 * @param task_id Identificador �nico de la tarea que ser� activada.
 *
 * @return OS_OK si suspendi� la tarea actual y activ� la otra tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador v�lido.
 */
error_id_e os_task_chain(task_id_t task_id);

void scheduler(void);

#endif /* OS_H_ */

/*
 * tasks.h
 *
 *  Created on: Oct 13, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_TASKS_H_
#define OS_TASKS_H_

#include "os.h"
#include "os_config.h"

typedef uint8_t os_task_id_t;
typedef void (*os_task_function_t)(void);

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
error_id_e os_task_create(os_task_id_t task_id, os_task_function_t task_function, uint8_t priority, uint8_t autostart);

/**
 * @brief Activa una tarea para que pueda ser elegida por el scheduler.
 *
 * @param task_id Identificador �nico de la tarea que ser� activada.
 *
 * @return OS_OK si activ� la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador v�lido.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanz� el l�mite de tareas activas.
 */
error_id_e os_task_activate(os_task_id_t task_id);

/**
 * @brief Activa una tarea desde una ISR y guarda el contexto de la tarea anterior.
 *
 * @param task_id Identificador �nico de la tarea que ser� activada.
 *
 * @return OS_OK si activ� la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador v�lido o si ya existe una tarea con ese ID.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanz� el l�mite de tareas activas.
 */
error_id_e os_task_activate_from_isr(os_task_id_t task_id);

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
error_id_e os_task_chain(os_task_id_t task_id);

#endif /* OS_TASKS_H_ */

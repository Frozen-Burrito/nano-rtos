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
 * @param task_id Identificador único de la tarea.
 * @param task_function Dirección de inicio de la tarea.
 * @param priority Prioridad de la tarea (0-255).
 * @param autostart Si es TRUE, inicializar el OS activa automáticamente la tarea.
 *
 * @return OS_OK si registró la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido.
 *         OS_ERROR_MAX_CREATED_TASKS si el OS alcanzó el límite de tareas creadas.
 */
error_id_e os_task_create(os_task_id_t task_id, os_task_function_t task_function, uint8_t priority, uint8_t autostart);

/**
 * @brief Activa una tarea para que pueda ser elegida por el scheduler.
 *
 * @param task_id Identificador único de la tarea que será activada.
 *
 * @return OS_OK si activó la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanzó el límite de tareas activas.
 */
error_id_e os_task_activate(os_task_id_t task_id);

/**
 * @brief Activa una tarea desde una ISR y guarda el contexto de la tarea anterior.
 *
 * @param task_id Identificador único de la tarea que será activada.
 *
 * @return OS_OK si activó la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido o si ya existe una tarea con ese ID.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanzó el límite de tareas activas.
 */
error_id_e os_task_activate_from_isr(os_task_id_t task_id);

/**
 * @brief Suspende una tarea.
 *
 * La tarea no vuelve a ser ejecutada hasta que se active otra vez.
 *
 * @return OS_OK si suspendió la tarea.
 */
error_id_e os_task_terminate(void);

/**
 * @brief Suspende la tarea actual y activa otra.
 *
 * @param task_id Identificador único de la tarea que será activada.
 *
 * @return OS_OK si suspendió la tarea actual y activó la otra tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido.
 */
error_id_e os_task_chain(os_task_id_t task_id);

#endif /* OS_TASKS_H_ */

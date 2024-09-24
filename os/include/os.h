/*
 * os.h
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_H_
#define OS_H_

#include <stdint.h>

typedef uint8_t task_id_t;
typedef void (*task_function_t)(void);

typedef enum _error_id_e {
    OS_OK,                          /* Ningún error, función completó con éxito. */
    OS_ERROR_INVALID_ARGUMENT,      /* Uno o varios de los argumentos de una función son no-válidos. */
    OS_ERROR_MAX_CREATED_TASKS,     /* El OS alcanzó el límite de tareas creadas. No se puede crear una nueva tarea. */
    OS_ERROR_MAX_ACTIVE_TASKS,      /* El OS ya tiene el máximo número de tareas activas. */
} error_id_e;

typedef enum _alarm_id_e {
    ALARM_A,
    ALARM_B,
    ALARM_MAX,
} alarm_id_e;

/**
 * @brief Inicializa el sistema operativo y ejecuta el scheduler por primera vez.
 *
 * @return OS_OK si inicializó OS sin errores.
 */
error_id_e os_init(void);

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
error_id_e os_task_create(task_id_t task_id, task_function_t task_function, uint8_t priority, uint8_t autostart);

/**
 * @brief Activa una tarea para que pueda ser elegida por el scheduler.
 *
 * @param task_id Identificador único de la tarea que será activada.
 *
 * @return OS_OK si activó la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanzó el límite de tareas activas.
 */
error_id_e os_task_activate(task_id_t task_id);

/**
 * @brief Activa una tarea desde una ISR y guarda el contexto de la tarea anterior.
 *
 * @param task_id Identificador único de la tarea que será activada.
 *
 * @return OS_OK si activó la tarea.
 *         OS_ERROR_INVALID_ARGUMENT si el valor de task_id no es un identificador válido o si ya existe una tarea con ese ID.
 *         OS_ERROR_MAX_ACTIVE_TASKS si se alcanzó el límite de tareas activas.
 */
error_id_e os_task_activate_from_isr(task_id_t task_id);

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
error_id_e os_task_chain(task_id_t task_id);

/**
 * @brief Activa una alarma relativa. Cuando la alarma relativa expira, el OS activa la tarea especificada.
 *
 * La alarma puede ser de activación única ("one-shot") o de activación múltiple ("auto-reload").
 *
 * @param id Identificador único de la alarma que será activada.
 * @param ticks Número de ticks para que expire la alarma, relativos al tick actual.
 * @param task_to_activate Identificador único de la tarea que será activada cuando la alarma expire.
 * @param autoreload Si es distinto de 0, el OS recarga la cuenta de la alarma y la vuelve a iniciar.
 *
 * @return OS_OK si el OS activó la alarma.
 *         OS_ERROR_INVALID_ARGUMENT si id, ticks o task_to_activate tienen valores inválidos.
 */
error_id_e os_alarm_set_rel(alarm_id_e id, uint16_t ticks, uint8_t task_to_activate, uint8_t autoreload);

/**
 * @brief Cancela una alarma, evitando que expire y active una tarea.
 *
 * @param id Identificador único de la alarma que será cancelada.
 *
 * @return OS_OK si el OS canceló la alarma.
 *         OS_ERROR_INVALID_ARGUMENT si id no es alguno de ALARM_*.
 */
error_id_e os_alarm_cancel(alarm_id_e id);

void scheduler(void);

#endif /* OS_H_ */

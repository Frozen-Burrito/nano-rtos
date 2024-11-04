/*
 * os.h
 *
 *  Created on: Aug 16, 2024
 *      Author: Fernando Mendoza V.
 */

#ifndef OS_H_
#define OS_H_

#include <stdint.h>

#include <msp430.h>

#include "em.h"
#include "hal_timer.h"

#include "os_config.h"

#define OS_MAX_TICKS    ((tick_type_t) 0xFFFFu)

typedef enum _error_id_e {
    OS_OK,                          /* Ningún error, función completó con éxito. */
    OS_ERROR_INVALID_ARGUMENT,      /* Uno o varios de los argumentos de una función son no-válidos. */
    OS_ERROR_MAX_CREATED_TASKS,     /* El OS alcanzó el límite de tareas creadas. No se puede crear una nueva tarea. */
    OS_ERROR_MAX_ACTIVE_TASKS,      /* El OS ya tiene el máximo número de tareas activas. */
    OS_ERROR_QUEUE_EMPTY,           /* No hay elementos para leer en la queue. */
    OS_ERROR_QUEUE_FULL,            /* La queue está llena, no puede agregar un nuevo elemento. */
} error_id_e;

typedef uint16_t tick_type_t;

/**
 * @brief Inicializa el sistema operativo y ejecuta el scheduler por primera vez.
 *
 * @return OS_OK si inicializó OS sin errores.
 */
error_id_e os_init(void);

/**
 * @brief Comienza la ejecución del scheduler del OS. Esta función no retorna.
 */
void scheduler_run(void);

#endif /* OS_H_ */

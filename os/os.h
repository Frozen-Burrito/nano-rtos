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

#include "hal_timer.h"

#include "os_config.h"

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
 * @brief Comienza la ejecuci�n del scheduler del OS. Esta funci�n no retorna.
 */
void scheduler_run(void);

#endif /* OS_H_ */
